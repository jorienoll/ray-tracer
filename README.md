# ray-tracer
This program is modified from the following  code:
Nate Robins, 1997, 2000
nate@pobox.com, http://www.pobox.com/~nate

This program helped me to familiariaze myself with the ray tracing algorithm.

First, I set up OpenGL and GLUT. In MacOS, I used XCode and created a command line tool in C++. Next, I added the OpenGL and GLUT frameworks to the project's frameworks and libraries.
I edited the Product Scheme to pass in the direct path of test.scene (edited for upload).

To start coding, I implemented the diffuse section of shade_ray_diffuse(). The diffuse color using l the diffuse equation. The ray color begins with c = (r, g, b), and then adds the color using the light diffusion and clamp the overflow into [0,1].

Next, I completed shade_ray_local(), which adds specular and shadow effects. Shading a ray begins with c = (r, g, b), which are the percieved material color, and s(l) being the color of light l. You then sum over all the lights l for each color chanel and clamp the overflow into [0,1]. Total color is computed by adding the ambient, diffuse, and specular of l. At each point, you apply the lighting model to get the color at that point and fill the pixel with that color.

Then I added sphere intersection testing in intersect_ray_sphere(). This function parameterizes the Intersection object returned like intersect_ray_glm_object() does. I also added a sphere in the file test.scene to see the results.The ray sphere intersection computations begin with the definition of a sphere |p - pc| - r^2 = 0, with a ray equation p = o + td. Combining these gives us |o + td - pc|^2 - r^2 = 0, where we solve for t. When nothing is hit witha. ray, use the background color.

Next, I completed the reflection component of shade_ray_recursive(). The reflected ray r can be calculated using the normal n and light direciton l in the equation r = 2(n * l)n - l.
We cannot use the formula for Phong illumination becuse incident ray v is pointing in the direction of the surface, whereas the light direction l is pointing away from the surface.  

Finally, I designed a complex and creative scene by modifying the file test.scene. I added several objects and spheres to demonstrate the shading, shadow, reflection, and reflaction functionality implemented in this program. I first added the three objects, al.obj, followed by rose+vase.obj, and lastly dolphins.obj.

This program resulted in a 250 by 250 image rendered that demonstrates the ray tracing algorithm, with diffuse and specular shading, shadows, reflections, and refractions on multiple objects and spheres.
