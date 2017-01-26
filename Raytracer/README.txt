CPSC453 - Assignment 4

NAME: NIZAR MAAN
UCID Number: 10103889
Tutorial Section:

=== 1. EXECUTION: ===

To compile the program, on the terminal enter the following commands:

	qmake -project QT+=widgets
	qmake
	make

To run the program, on the terminal enter the following command:

	./a4

=== 2. PROGRAM USE: ===
-Open Linux Terminal
-Go to project directory (via cd <directoy> command)
-once in project directoy enter ./a4 <imageWidth> <imageHeight>
-you will be prompted for an image name
-image will be generated and saved in project directory
-enjoy

== 3. ALGORITHMS and DESIGN DECISIONS: ===

Most algorithms were described in lecture notes, others are based off of algorithms found on wikipedia.

algebra.cpp/h were used for Vector3D and Point3D arithmatic 

our scene, objects, light, material are all classes outside of main, all calculations are done within Scene.

Main simply calculates pixel positions and sends the ray through that pixel to be traced.

we call the trace() function which is a function within the Scene() class, which returns a color value which is used to set that particular pixel color


/****************** IMPLEMENTATION TEST ********************/
//raytracer based off of http://pages.cpsc.ucalgary.ca/~eharris/cpsc453/tut17/
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            float delta; //var for anti aliasing
            double X = xMin + x*deltaX;
            double Y = yMax - y*deltaY;
            double Z = 0;

            Point3D p = Point3D(X, Y, Z);


            Vector3D d = p - scene.eye;
            d.normalize();
            QColor color = scene.trace(scene.eye, d, depth);

            // set pixel value
            image.setPixel(x, y,
                qRgb(color.red(), color.green(),color.blue()));
        }
    }

algorithm that traces a ray:
//trace based off of course notes: https://d2l.ucalgary.ca/d2l/le/content/106725/viewContent/1803013/View
we get the eye point (camera point) which is the origin of the ray, and its direction.
we send these two variables to our sphereintersect() function which uses these two variables along with sphere formula to find the smallest positive root. 
 q = intersect(p, d); where p is the ray origin and d its direction a Point3D and Vector3D respectively
this root is used to find the intersection point of the ray
with the sphere. we then calculate the normal of the sphere by subtracting its center from the intersection point (Q - C).
 //http://pages.cpsc.ucalgary.ca/~eharris/cpsc453/tut17/ was used for sphere formula
 
     Vector3D sphereNormal(Point3D p, int sphereID){
        Vector3D ret = p - objects[sphereID]->center;
        ret.normalize();
        return ret;
    }
	
If the sphere was hit within the intersection() function, a boolean variable is set reflecting this fact.
The following steps are taken within Trace() if the sphere is hit after the intersection() check:

        if(sphere1Hit == true){
            n = sphereNormal(q, 0);

            q = q + (0.1*n); //correction to intersection point 'q'

            r = reflect(d, n);
            r.normalize();
            //passing sphere material
            local = phong(q, n, objects[0]->mat, 0);
            reflected = trace(q, r, depth+1);
        }
		
where q is the intersection point with the sphere, and local, and reflected are colors, n the sphere normal with respect to the intersection point and r is reflection vector
q = q + (0.1*n); //correction to intersection point 'q' -> we make this slight adjustment to q.	
We get the normal, calculate a reflection vector, normalize it, and recursively call trace with this new reflection vector until our DEPTH cap is reached
We also get the phong color contribution which is calculated like so:

//phong based off of course notes: https://d2l.ucalgary.ca/d2l/le/content/106725/viewContent/1803013/View
 for(int i = 0; i < lights.size(); i++){
            Vector3D shadowRay = (lights[i]->pos - p);
            shadowRay.normalize();
            intersect(p, shadowRay);

            //sphere 1
            if(objectID == 0){
                if(sphere1Hit == false){
                    //diffuse
                    ret+= material->kd * lights[i]->Id * qMax(n.dot(shadowRay), (double) 0);

                    //specular
                    Vector3D R = 2*(n.dot(shadowRay)) * n - shadowRay;
                    Vector3D V = eye - p;
                    R.normalize();
                    V.normalize();
                    ret+= material->ks * lights[i]->Is * qPow(qMax(R.dot(V), (double) 0), material->exp);
                }
            }
			ret = clamp(ret);

        QColor col;
        //if a sphere's being shaded, shade it this color
        if(objectID == 0){
            col = QColor(0,ret,ret, 0);
        }
		
		return col;

each object in the scene has an arbirary ID#, in this example sphere 1 has ID 0, we calculate its diffuse and specular contributions according to arbitrary material and light coefficients

we get back a color value as a double, and clamp it between values 0 and 255
depending on the object ID, we set it to an arbitrarily chosen color using the calculated ret value. 

triangle ray intersection formulas were gathered and modified from:
//triangle intersections from:
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
=== 4. FILES SUBMITTED: ===
a4
a4.pro
a4.pro.user
algebra.cpp
algebra.h
algebra.o
main.cpp
main.o
Makefile
Makefile.debug
Makefile.release
polyroots.cpp
polyroots.h
polyroots.o
README.txt
test.png
nizar.maan_a4_1.png

=== 5. IMAGES SUBMITTED: ===

nizar.maan_a4_1.png

this image shows 2 spheres with phong shading, reflection and slight shadows, as well as a few triangles.

=== 6. PROGRAM ASSUMPTIONS: ===

I assume anyone using this program is familiar with linux command line,Qt and c++. I assume this person has basic knowledge of computers and pixel dimensions of a picture.

=== 7. DECLARATION AND IMAGE PERMISSION: ===

<Initial the following:>

_NM_ I have read the regulations regarding plagarism. I affirm with my initials
that I have worked on my own solution to this assignment, and the code I am
submitting is my own.

<Mark with an X if desired. No marks will be deducted either way>

X__ I grant the instructor and/or TAs permission to use images submitted and/or
   assignment screenshots for future CPSC453 courses.

__ If permission has been granted above, I would prefer the images to remain 
   annonymous. <Otherwise credit will be given, as is due>

__ I would like to participate in the raytracer competition. <Competition entries will
   not be annonymous and credited accordingly. Specify which images are for consideration,
   otherwise all will be considered.> 

__
