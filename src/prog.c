#include "prog.h"
#include "util.h"
#include "texture.h"
#include <stdlib.h>
#include <cglm/cglm.h>

bool g_focus_back = false;
struct Prog *prog_alloc(GLFWwindow *win)
{
    struct Prog *p = malloc(sizeof(struct Prog));
    p->running = true;
    p->focused = true;
    p->win = win;

    p->cam = cam_alloc((vec3){ 0.f, 0.f, 0.f }, (vec3){ 0.f, 0.f, 0.f });

    p->ri = ri_alloc();
    ri_add_shader(p->ri, SHADER_BASIC, "shaders/basic_v.glsl", "shaders/basic_f.glsl");

    p->ri->cam = p->cam;

    return p;
}


void prog_free(struct Prog *p)
{
    cam_free(p->cam);
    ri_free(p->ri);
    free(p);
}


void prog_mainloop(struct Prog *p)
{
    glEnable(GL_DEPTH_TEST);
    /* glEnable(GL_CULL_FACE); */

    glfwSetCursorPos(p->win, SCRW / 2.f, SCRH / 2.f);
    /* glfwSetInputMode(p->win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); */

    glfwGetCursorPos(p->win, &p->prev_mx, &p->prev_my);

    float ratio = 940.f / 593.f; // w / h
    float oor = 1.f / ratio; // h / w

    vec3 pverts[] = {
        { 20.f, 0.f, 0.f }, // top left
        { 20.f, oor * -10.f, 0.f }, // bottom left
        { 20.f, oor * -10.f, ratio * 10.f }, // bottom right

        { 20.f, 0.f, ratio * 10.f }, // top right
        { 20.f, 0.f, 0.f }, // top left
        { 20.f, oor * -10.f, ratio * 10.f } // bottom right
    };

    float verts[] = {
        /* 20.f, 0.f, 0.f, 0.f, 0.f, */
        /* 20.f, -10.f, 0.f, 0.f, 1.f, */
        /* 20.f, -10.f, 10.f, 1.f, 1.f */
        20.f, 0.f, 0.f,                  0.f, 0.f, // top left
        20.f, oor * -10.f, 0.f,          0.f, 1.f,// bottom left
        20.f, oor * -10.f, ratio * 10.f, 1.f, 1.f, // bottom right

        20.f, 0.f, ratio * 10.f,         1.f, 0.f, // top right
        20.f, 0.f, 0.f,                  0.f, 0.f, // top left
        20.f, oor * -10.f, ratio * 10.f, 1.f, 1.f // bottom right
    };

    unsigned int vao, vb;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    struct Texture *tex = tex_alloc("out.png");

    system("python3 get.py 100 100 &");

    int scrx = 1, scry = 1;

    while (p->running)
    {
        FILE *f = fopen("ready", "r");
        if (f)
        {
            fclose(f);
            tex_free(tex);
            tex = tex_alloc("out.png");
            glfwGetCursorPos(p->win, &p->prev_mx, &p->prev_my);
        }

        if (p->focused)
        {
            double mx, my;
            glfwGetCursorPos(p->win, &mx, &my);

            cam_rot(p->cam, (vec3){ 0.f, -(my - p->prev_my) / 100.f, -(mx - p->prev_mx) / 100.f });
            p->prev_mx = mx;
            p->prev_my = my;
        }

        {
            FILE *tmp = fopen("mcoords_ready", "r");
            if (tmp)
            {
                fclose(tmp);
                system("rm mcoords_ready");
            }

            if (scrx > 0 && scry > 0 && scrx < 940 && scry < 593)
            {
                FILE *mc = fopen("mcoords", "w");
                fprintf(mc, "%d %d", scrx, scry);
                fclose(mc);
            }

            FILE *ready = fopen("mcoords_ready", "w");
            fclose(ready);
        }
        {
            FILE *click = fopen("click", "r");
            if (!click && g_focus_back)
            {
                glfwMakeContextCurrent(p->win);
                p->focused = true;
                /* glfwSetInputMode(p->win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); */
                g_focus_back = false;
            }
        }

        prog_events(p);

        float d1, d2;
        bool i1 = glm_ray_triangle(p->cam->pos, p->cam->front, pverts[0], pverts[1], pverts[2], &d1);
        bool i2 = glm_ray_triangle(p->cam->pos, p->cam->front, pverts[3], pverts[4], pverts[5], &d2);
        float d = INFINITY;

        if (i1) d = d1;
        else if (i2) d = d2;

        if (d != INFINITY)
        {
            vec3 intersection;
            glm_vec3_scale(p->cam->front, d, intersection);
            glm_vec3_add(p->cam->pos, intersection, intersection);
            glm_vec3_sub(intersection, pverts[0], intersection);

            float max_z = ratio * 10.f;
            float min_y = -oor * 10.f;
            scrx = (intersection[2] / max_z) * 940.f;
            scry = (intersection[1] / min_y) * 593.f;
            printf("%f %d %d\n", intersection[2], scrx, scry);
        }

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ri_use_shader(p->ri, SHADER_BASIC);

        cam_set_props(p->cam, p->ri->shader);
        cam_view_mat(p->cam, p->ri->view);

        shader_mat4(p->ri->shader, "view", p->ri->view);
        shader_mat4(p->ri->shader, "projection", p->ri->proj);

        mat4 model;
        glm_mat4_identity(model);
        shader_mat4(p->ri->shader, "model", model);

        tex_bind(tex, 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(p->win);
        glfwPollEvents();
    }

    tex_free(tex);
}


void prog_events(struct Prog *p)
{
    if (glfwWindowShouldClose(p->win)) p->running = false;

    if (p->focused && glfwGetKey(p->win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        /* glfwSetInputMode(p->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL); */
        p->focused = false;
        system("pkill python");
    }

    if (!p->focused && glfwGetMouseButton(p->win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwGetCursorPos(p->win, &p->prev_mx, &p->prev_my);
        /* glfwSetInputMode(p->win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); */
        p->focused = true;
        system("python3 get.py &");
    }

    static float last_click = -100.f;

    if (glfwGetKey(p->win, GLFW_KEY_Q) == GLFW_PRESS && glfwGetTime() - last_click > 1.f)
    {
        p->focused = false;
        glfwSetInputMode(p->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        FILE *f = fopen("click", "w");
        fclose(f);
        last_click = glfwGetTime();
        g_focus_back = true;
    }


    float move = .05f;

    vec3 angle;
    glm_vec3_copy(p->cam->rot, angle);
    angle[1] = 0.f;

    vec4 quat;
    util_eul2quat(angle, quat);

    vec3 front = { 1.f, 0.f, 0.f };
    glm_quat_rotatev(quat, front, front);
    glm_vec3_scale(front, move, front);

    vec3 right = { 0.f, 0.f, 1.f };
    glm_quat_rotatev(quat, right, right);
    glm_vec3_scale(right, move, right);

    if (glfwGetKey(p->win, GLFW_KEY_W) == GLFW_PRESS) glm_vec3_add(p->cam->pos, front, p->cam->pos);
    if (glfwGetKey(p->win, GLFW_KEY_S) == GLFW_PRESS) glm_vec3_sub(p->cam->pos, front, p->cam->pos);
    if (glfwGetKey(p->win, GLFW_KEY_A) == GLFW_PRESS) glm_vec3_sub(p->cam->pos, right, p->cam->pos);
    if (glfwGetKey(p->win, GLFW_KEY_D) == GLFW_PRESS) glm_vec3_add(p->cam->pos, right, p->cam->pos);

    if (glfwGetKey(p->win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) p->cam->pos[1] -= move;
    if (glfwGetKey(p->win, GLFW_KEY_SPACE) == GLFW_PRESS) p->cam->pos[1] += move;
}

