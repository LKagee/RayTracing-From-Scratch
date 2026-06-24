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
	int specular;

} Sphere;

typedef struct {
        vec3 direction;
	double intensity;
 
} Directional_Light;


typedef struct {
	double closest_t;
	Sphere *closest_sphere;
} Pack;



color3 background_color = {53, 81, 92};


// Optimized function using the new hardcoded look-down-and-right matrix
  vec3 multiplyMatrixVector(vec3 v) {
    // New matrix coefficients for Camera (-3,0,2) pointing at (0,-2,1):
    // Row 1 (Right):   [-0.55470020, -0.83205029,  0.00000000]
    // Row 2 (Up):      [ 0.22237479, -0.14824986,  0.96362411]
    // Row 3 (Forward): [ 0.80178373, -0.53452248, -0.26726124]
    
    vec3 result;
    result.x = -0.55470020 * v.x - 0.83205029 * v.y;
    result.y =  0.22237479 * v.x - 0.14824986 * v.y + 0.96362411 * v.z;
    result.z =  0.80178373 * v.x - 0.53452248 * v.y - 0.26726124 * v.z;
    return result;
}



vec3 CanvasToViewport(double x, double y) {

	vec3 ret = {x*CANVAS_TO_VIEW_W, y*CANVAS_TO_VIEW_H, 1};
	return ret;
}

double dot(vec3 u, vec3 v) {
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

double length(vec3 v) {
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
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

Pack* ClosestIntersection(vec3 origin, vec3 direction, Sphere* spheres, double t_min, double t_max) {
	Sphere *closest_sphere = NULL;
	double closest_t = INFINITY;	
	for(int i = 0; i < 4; i++) {

	double* result = RayIntersect_Sphere(direction, origin, spheres[i]);


	

	if(result == NULL) {
		continue;
	}
	
	if(result[0] < t_min || result[1] < t_min) continue;
	if(result[0] > t_max || result[1] > t_max) continue;

	if(result[0] < closest_t) {
		closest_t = result[0];
		closest_sphere = &spheres[i];
	}

	if(result[1] < closest_t) {
		closest_t = result[1];
		closest_sphere = &spheres[i];
	}
}

Pack ret = {.closest_t = closest_t, .closest_sphere = closest_sphere};
Pack *t_ret = &ret;
return t_ret;
       
}

double ComputeLight(vec3 N, vec3 P, vec3 V, Directional_Light *light, Sphere *spheres, int s) {

	double i = 0.2;
	for(int k = 0; k < 2; k++) {

	vec3 L = {light[k].direction.x - P.x, light[k].direction.y - P.y, light[k].direction.z - P.z};

	Pack *shadow_sphere = ClosestIntersection(P, L, spheres, .001, INFINITY);
	if(shadow_sphere->closest_sphere != NULL) continue;


	double i_dot = dot(N, L);
	if(i_dot < 0) i_dot = 0;
	i += light[k].intensity * i_dot / ( length(N) * length(L) );



	if(s != -1) {
		double dot_d = dot(N, L);
		
		vec3 R = {2*N.x*dot_d - L.x, 2*N.y*dot_d - L.y, 2*N.z*dot_d - L.z};
		double r_dot = dot(R, V);
		if(r_dot > 0) i+=light[k].intensity * pow(r_dot/(length(R)*length(V)), s);


	}
}
	if(i > 1) i = 1;
	return i;




		
}

color3 TraceRay(vec3 ray, vec3 camera, Sphere* spheres) {

	Pack *test = ClosestIntersection(camera, ray, spheres, 0, INFINITY);
	if(test->closest_sphere == NULL) {
		return background_color;
	}

	vec3 P = {camera.x+(test->closest_t*ray.x), camera.y+(test->closest_t*ray.y), camera.z+(test->closest_t*ray.z)};
	vec3 N = {P.x-test->closest_sphere->center.x, P.y-test->closest_sphere->center.y, P.z-test->closest_sphere->center.z};
        vec3 new_N = {N.x / length(N), N.y / length(N), N.z / length(N)};
        N = new_N;
	Directional_Light light = {.direction.x = -1, .direction.y = 1, .direction.z = 3, .intensity = .6};
	Directional_Light light2 = {.direction.x = 5, .direction.y = 0, .direction.z = 1, .intensity = 0};
	Directional_Light light_arr[2] = {light, light2};

	vec3 inv_D = {-ray.x, -ray.y, -ray.z};
	double light_int = ComputeLight(N, P, inv_D, light_arr, spheres, test->closest_sphere->specular);


	color3 return_col = {test->closest_sphere->color.r * light_int, test->closest_sphere->color.g * light_int, test->closest_sphere->color.b * light_int};
        
	
	return return_col;
}





void PutPixel(color3 color, FILE* file) {

	if(color.r > 255) color.r = 255;
	if(color.g > 255) color.g = 255;
	if(color.b > 255) color.b = 255;
		
	fprintf(file, "%d %d %d\n", color.r, color.g, color.b);	
}

vec3 camera = {0, 0, 1};


int main( void )
{
	Sphere sphere;
    sphere.color.r = 255;
    sphere.color.g = 0;
    sphere.color.b = 0; 
    sphere.center.x = -1;
    sphere.center.y = 0;
    sphere.center.z = 6; 
    sphere.radius = 1;
    sphere.specular = 100;

	Sphere sphere2;
    sphere2.color.r = 0;
    sphere2.color.g = 255;
    sphere2.color.b = 0; 
    sphere2.center.x = -5;
    sphere2.center.y = 0;
    sphere2.center.z = 10; 
    sphere2.radius = 1;
    sphere2.specular = -1;

    	Sphere sphere3;
    sphere3.color.r = 0;
    sphere3.color.g = 0;
    sphere3.color.b = 255; 
    sphere3.center.x = 1;
    sphere3.center.y = 0;
    sphere3.center.z = 4; 
    sphere3.radius = 1;
    sphere3.specular = 10;

    Sphere sphere4 = {.color.r = 255, .color.g = 255, .color.b = 0, .center.x = 0, .center.y = -5001, .center.z = 0, .radius = 5000, .specular = -1};


    Sphere sphere_arr[4];
    sphere_arr[0] = sphere;
    sphere_arr[1] = sphere2;
    sphere_arr[2] = sphere3;
    sphere_arr[3] = sphere4;

	
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

