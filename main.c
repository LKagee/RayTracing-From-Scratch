#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define CANVAS_HEIGHT 420
#define CANVAS_WIDTH 640

#define VIEWPORT_HEIGHT 1.0
#define VIEWPORT_WIDTH (VIEWPORT_HEIGHT * ((double)CANVAS_WIDTH / CANVAS_HEIGHT))

#define CANVAS_TO_VIEW_W (VIEWPORT_WIDTH/CANVAS_WIDTH)
#define CANVAS_TO_VIEW_H (VIEWPORT_HEIGHT/CANVAS_HEIGHT)


typedef struct {
	double x, y, z;
} vec3;

typedef struct {
	int r, g, b;
} color3;
typedef struct {
	vec3 center;
	color3 color;
	double radius;

} Sphere;

color3 background_color = {255, 255, 255};

vec3 CanvasToViewport(double x, double y) {

	vec3 ret = {x*CANVAS_TO_VIEW_W, y*CANVAS_TO_VIEW_H, 1};
	return ret;
}

double dot(vec3 u, vec3 v) {
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

double* RayIntersect_Sphere(vec3 ray, vec3 camera, Sphere sphere) {
	vec3 test = {camera.x - sphere.center.x, camera.y - sphere.center.y, camera.z - sphere.center.z};
	static double t[2];
	double a = dot(ray, ray);
	double b = 2*dot(test, ray);
	double c = dot(test, test) - sphere.radius*sphere.radius;

	double discriminant = b*b - 4*a*c;
	if(discriminant < 0) return NULL;
 	double t1 = (-b + sqrt(discriminant)) / (2*a);
	double t2 = (-b - sqrt(discriminant)) / (2*a);
	t[0] = t1;
	t[1] = t2;
	return t;
}

color3 TraceRay(vec3 ray, vec3 camera, Sphere* spheres) {

	Sphere* closest_sphere = NULL;
	double closest_t = INFINITY;
	for(int i = 0; i < 3; i++) {

	double* result = RayIntersect_Sphere(ray, camera, spheres[i]);

	

	if(result == NULL) {
		continue;
	}


	if(result[0] < closest_t) {
		closest_t = result[0];
		closest_sphere = &spheres[i];
	}

	if(result[1] < closest_t) {
		closest_t = result[1];
		closest_sphere = &spheres[i];
	}
}
       

	if(closest_sphere == NULL) {
		return background_color;
	}
	return closest_sphere->color;
}

void PutPixel(color3 color, FILE* file) {
		
	fprintf(file, "%d %d %d\n", color.r, color.g, color.b);	
}

vec3 camera = {0, 0, 1};


int main( void )
{
	Sphere sphere;
    sphere.color.r = 255;
    sphere.color.g = 0;
    sphere.color.b = 0; 
    sphere.center.x = 0;
    sphere.center.y = 0;
    sphere.center.z = 5; 
    sphere.radius = 1;

	Sphere sphere2;
    sphere2.color.r = 0;
    sphere2.color.g = 255;
    sphere2.color.b = 0; 
    sphere2.center.x = -1;
    sphere2.center.y = 0;
    sphere2.center.z = 6; 
    sphere2.radius = 1;

    	Sphere sphere3;
    sphere3.color.r = 0;
    sphere3.color.g = 0;
    sphere3.color.b = 255; 
    sphere3.center.x = 1;
    sphere3.center.y = 0;
    sphere3.center.z = 4; 
    sphere3.radius = 1;


    Sphere sphere_arr[3];
    sphere_arr[0] = sphere;
    sphere_arr[1] = sphere2;
    sphere_arr[2] = sphere3;

	
	FILE* file = fopen("image.ppm", "w");
	if(file == NULL) {
		printf("\nError opening ppm file\n");
	       	return 0;
	}
	fprintf(file, "P3\n%d %d\n255\n", CANVAS_WIDTH, CANVAS_HEIGHT);
	for(int y = CANVAS_HEIGHT/2; y > -CANVAS_HEIGHT/2; --y) {
		for(int x = -CANVAS_WIDTH/2; x < CANVAS_WIDTH/2; x++) {
			vec3 ray = CanvasToViewport(x, y);
			color3 color = TraceRay(ray, camera, sphere_arr);
			PutPixel(color, file);
		}
	}
	fclose(file);
	return 0;
}

