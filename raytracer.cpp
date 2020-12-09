// Ray Tracing

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/GLUT.h>“
#include "myray.h"
#include "glm.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

extern Camera* ray_cam;       // camera info
extern int image_i, image_j;  // current pixel being shaded
extern bool wrote_image;      // has the last pixel been shaded?

							  // reflection/refraction recursion control

extern int maxlevel;          // maximum depth of ray recursion 
extern double minweight;      // minimum fractional contribution to color

							  // these describe the scene

extern vector < GLMmodel* > model_list;
extern vector < Sphere* > sphere_list;
extern vector < Light* > light_list;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// intersect a ray with the entire scene (.obj models + spheres)

// x, y are in pixel coordinates with (0, 0) the upper-left hand corner of the image.
// color variable is result of this function--it carries back info on how to draw the pixel

void trace_ray(int level, double weight, Ray* ray, Vect color)
{
	Intersection* nearest_inter = NULL;
	Intersection* inter = NULL;
	int i;

	// test for intersection with all .obj models

	for (i = 0; i < model_list.size(); i++) {
		inter = intersect_ray_glm_object(ray, model_list[i]);
		update_nearest_intersection(&inter, &nearest_inter);
	}

	// test for intersection with all spheres

	for (i = 0; i < sphere_list.size(); i++) {
		inter = intersect_ray_sphere(ray, sphere_list[i]);
		update_nearest_intersection(&inter, &nearest_inter);
	}

	// "color" the ray according to intersecting surface properties

	// choose one of the simpler options below to debug or preview your scene more quickly.
	// another way to render faster is to decrease the image size.

	if (nearest_inter) {
		//shade_ray_false_color_normal(nearest_inter, color);
		//  shade_ray_intersection_mask(color);  
		shade_ray_diffuse(ray, nearest_inter, color);
		//   shade_ray_recursive(level, weight, ray, nearest_inter, color);
	}

	// color the ray using a default

	else
		shade_ray_background(ray, color);
}

//----------------------------------------------------------------------------

// test for ray-sphere intersection; return details of intersection if true
Intersection* intersect_ray_sphere(Ray* ray, Sphere* S)
{
    Vect V;
    Vect N;
    Vect ret;

    Intersection* inter;

    VectSub(ray->orig, S->P, V);

    double x = VectDotProd(ray->dir, V);
    double descrim = pow(x, 2) - VectDotProd(V, V) + pow(S->radius, 2);
    inter = make_intersection();

    if(descrim < 0){
        return NULL;
    }
    else if (descrim == 0){
        inter->t = x * -1;
        ret[X] = inter->t + ray->orig[X] * ray->dir[X];
        ret[Y] = inter->t + ray->orig[Y] * ray->dir[Y];
        ret[Z] = inter->t + ray->orig[Z] * ray->dir[Z];
        VectCopy(inter->P, ret);
        
        // compute sphere intersection with normal
        N[X] = (inter->P[X] - S->P[X]) / S->radius;
        N[Y] = (inter->P[Y] - S->P[Y]) / S->radius;
        N[Z] = (inter->P[Z] - S->P[Z]) / S->radius;
        VectCopy(inter->N, N);
        
        // compute sphere intersection with surface
        inter->surf = S->surf;
    }
    else if (descrim > 0){
        double x1 = -x + sqrt(descrim);
        double x2 = -x - sqrt(descrim);
        double t;
        
        if (x1 < 0){
            if (x2 < 0)
                return NULL;
            t = x2;
        }
        else if (x2 < 0){
            if (x1 < 0)
                return NULL;
            t = x1;
        }
        else {
            t = (x1 > x2) ? x2 : x1;
        }
        
        inter->t = t;
        ret[X] = t + ray->orig[X] * ray->dir[X];
        ret[Y] = t + ray->orig[Y] * ray->dir[Y];
        ret[Z] = t + ray->orig[Z] * ray->dir[Z];
        VectCopy(inter->P, ret);
        
        // compute sphere intersection with normal
        N[X] = (inter->P[X] - S->P[X]) / S->radius;
        N[Y] = (inter->P[Y] - S->P[Y]) / S->radius;
        N[Z] = (inter->P[Z] - S->P[Z]) / S->radius;
        VectCopy(inter->N, N);
        
        // compute sphere intersection with surface
        inter->surf = S->surf;
    }

    return inter;
}

//----------------------------------------------------------------------------

// only local, ambient + diffuse lighting (no specular, shadows, reflections, or refractions)
void shade_ray_diffuse(Ray* ray, Intersection* inter, Vect color){
	Vect L;
	double diff_factor;

	// iterate over lights

	for (int i = 0; i < light_list.size(); i++) {

		// AMBIENT

		color[R] += inter->surf->amb[R] * light_list[i]->amb[R];
		color[G] += inter->surf->amb[G] * light_list[i]->amb[G];
		color[B] += inter->surf->amb[B] * light_list[i]->amb[B];

		// DIFFUSE
    
        VectSub(light_list[i]->P, inter->P, L);
        VectUnit(L);
        diff_factor = VectDotProd(inter->N, L);
        diff_factor = diff_factor > 0 ? diff_factor : 0;
        
        color[R] += inter->surf->diff[R] * light_list[i]->diff[R] * diff_factor;
        color[G] += inter->surf->diff[G] * light_list[i]->diff[G] * diff_factor;
        color[B] += inter->surf->diff[B] * light_list[i]->diff[B] * diff_factor;
	}

	// clamp color to [0, 1]

	VectClamp(color, 0, 1);
}

//----------------------------------------------------------------------------

// traces the ray shadow
bool trace_shadow(Ray *ray){
    bool h = false;
    int i;
    
    //find if ray interesect with all .obj models
    while (!h && i < model_list.size()) {
        if (intersect_ray_glm_object(ray, model_list[i]))
            h = true;
        i++;
    }
    
    i = 0;

    //find if ray intersect with all spheres
    while(!h && i < sphere_list.size()) {
        if (intersect_ray_sphere(ray, sphere_list[i]) != NULL){
            h = true;
        }
        i++;
    }
    
    return h;
}

// if obj is in shadow
bool shadow(Intersection *inter, Light* light){
    Vect origin;
    Vect d;
    
    double e = 1.0e-3;
    Ray* shadow;
    
    VectSub(light->P, inter->P, d);
    VectUnit(d);

    origin[X] = inter->P[X] + e * d[X];
    origin[Y] = inter->P[Y] + e * d[Y];
    origin[Z] = inter->P[Z] + e * d[Z];

    shadow = make_ray(origin,d);
    
    return trace_shadow(shadow);
}


// same as shade_ray_diffuse(), but add specular lighting + shadow rays (i.e., full Phong illumination model)
void shade_ray_local(Ray* ray, Intersection* inter, Vect color){
    Vect L;
    Vect H;
    Vect eye;

    double diff_factor;
    double diff_factor_H;
    
    for(int i = 0; i < light_list.size(); i++){
            if (shadow(inter, light_list[i])) {
                //ambient
                color[R] += inter->surf->amb[R] * light_list[i]->amb[R];
                color[G] += inter->surf->amb[G] * light_list[i]->amb[G];
                color[B] += inter->surf->amb[B] * light_list[i]->amb[B];
            }
            else {
                //ambient
                color[R] += inter->surf->amb[R] * light_list[i]->amb[R];
                color[G] += inter->surf->amb[G] * light_list[i]->amb[G];
                color[B] += inter->surf->amb[B] * light_list[i]->amb[B];
                
                //diffuse
                VectSub(light_list[i]->P, inter->P, L);
                VectUnit(L);
                diff_factor = VectDotProd(inter->N, L);
                diff_factor = diff_factor > 0 ? diff_factor : 0;
                
                color[R] += inter->surf->diff[R] * light_list[i]->diff[R] * diff_factor;
                color[G] += inter->surf->diff[G] * light_list[i]->diff[G] * diff_factor;
                color[B] += inter->surf->diff[B] * light_list[i]->diff[B] * diff_factor;
                
                //specular
                VectSub(ray_cam->eye, inter->P, eye);
                VectSub(light_list[i]->P, inter->P, L);
                VectUnit(eye);
                VectUnit(L);
                VectAddS(1.0, L, eye, H);
                VectUnit(H);

                diff_factor_H = VectDotProd(inter->N, H);
                diff_factor_H = diff_factor_H > 0 ? diff_factor_H : 0;

                color[R] += inter->surf->spec[R] * light_list[i]->spec[R] * pow(diff_factor_H, inter->surf->spec_exp);
                color[G] += inter->surf->spec[G] * light_list[i]->spec[G] * pow(diff_factor_H, inter->surf->spec_exp);
                color[B] += inter->surf->spec[B] * light_list[i]->spec[B] * pow(diff_factor_H, inter->surf->spec_exp);
            }
        }
        VectClamp(color, 0, 1);
}

//----------------------------------------------------------------------------

// full shading model: ambient/diffuse/specular lighting, shadow rays, recursion for reflection, refraction

// level = recursion level (only used for reflection/refraction)

void shade_ray_recursive(int level, double weight, Ray* ray, Intersection* inter, Vect color)
{
    Vect direction;
    Vect position;
    double e = 1.0e-9;

    Surface* surf = (Surface*)malloc(sizeof(Surface));
    Ray* reflection;

	// initialize color to Phong reflectance model
	shade_ray_local(ray, inter, color);

	// if not too deep, recurse
	if (level + 1 < maxlevel) {
        
		// add reflection component to color
		if (weight * surf->reflectivity > minweight){
            reflection_direction(ray->dir, inter->N, direction);
            VectUnit(direction);

            position[X] += direction[X] * e;
            position[Y] += direction[Y] * e;
            position[Z] += direction[Z] * e;

            reflection = make_ray(position, direction);
            trace_ray(level + 1, weight, reflection, color);
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// ray trace another pixel if the image isn't finished yet
void idle()
{
	if (image_j < ray_cam->im->h) {

		raytrace_one_pixel(image_i, image_j);

		image_i++;

		if (image_i == ray_cam->im->w) {
			image_i = 0;
			image_j++;
		}
	}

	// write rendered image to file when done

	else if (!wrote_image) {

		write_PPM("output.ppm", ray_cam->im);

		wrote_image = true;
	}

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

// show the image so far

void display(void)
{
	// draw it!

	glPixelZoom(1, -1);
	glRasterPos2i(0, ray_cam->im->h);

	glDrawPixels(ray_cam->im->w, ray_cam->im->h, GL_RGBA, GL_FLOAT, ray_cam->im->data);

	glFlush();
}

//----------------------------------------------------------------------------

void init()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, ray_cam->im->w, 0.0, ray_cam->im->h);
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// initialize scene (must be done before scene file is parsed)
	Camera* new_cam = (Camera*)malloc(sizeof(Camera));
	ray_cam = (Camera*)malloc(sizeof(Camera));
	init_raytracing(ray_cam);



	if (argc == 2)
		parse_scene_file(argv[1], ray_cam);
	else {
		printf("missing .scene file\n");
		exit(1);
	}

	// opengl business
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(ray_cam->im->w, ray_cam->im->h);
	glutInitWindowPosition(500, 300);
	glutCreateWindow("hw4_raytracing");
	init();

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();

	return 0;
}

//----------------------------------------------------------------------------
