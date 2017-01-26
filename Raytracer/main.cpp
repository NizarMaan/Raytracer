#include <QImage>
#include <QColor>
#include <QtMath>
#include <QDebug>
#include <QtGlobal>
#include "algebra.h"


/*This program is a raytracer that generates and .png file and saves it to the project's directory
 * the image genrated is 2 spheres that reflect light and generate specular highlight, a triangle and a quadrilaterall with no light
 * properties*/

class Light{
public:
    Point3D pos;
    QColor c;
    double Is;
    double Id;

    Light(){
        pos = Point3D(1000, 1050, -1000);
        Is = 0.9;
        Id = 1.0;
    }
};

class Material{
public:
    QColor c;
    double ka, kd, ks, exp;
};

class Object{
public:
    Material * mat;

    //sphere attributes
    Point3D center;
    float radius;

    Point3D v1,v2,v3;  //triangle vert points
    Vector3D triNorm;
};

//setup a sphere's coordinates, child of object class, set up its material properties
class Sphere:public Object{
public:
    //(y cord is inverted)
    Sphere(){
        center = Point3D(250, 250, 0);
        radius = 120;
        mat = new Material();
        mat->ka = 170;
        mat->kd = 170;
        mat->ks = 90;
        mat->exp = 170;
    }
};

//setup triangle's material properties and coordinates
class Triangle:public Object{
public:
    Triangle(){
        //defined counter-clock wise
        //vert 1 (A) (y cord is inverted)
        v1 = Point3D(75, 200, -10);

        //vert 2 (B)
        v2 = Point3D(50, 100, -10);

        //vert 3 (C)
        v3 = Point3D(100, 100, -10);

        mat = new Material();
        mat->ka = 170;
        mat->kd = 170;
        mat->ks = 105;
        mat->exp = 170;
    }
};

class Scene{
public:
    Point3D eye;
    Point3D imgMin, imgMax;
    int width, height;
    int MAX_DEPTH;
    double GLOBAL_AMBIENT;
    QColor bgColor;
    bool tri1Hit;        //triangle intersection status
    bool tri2Hit;        //triangle intersection status
    bool tri3Hit;
    bool sphere1Hit;    //sphere intersection status for first sphere
    bool sphere2Hit;    //sphere intersection status for second sphere
    int i;              //counter of intersection hits for debugging

    std::vector<Light*> lights;
    std::vector<Object*> objects;

    Scene(){
        tri1Hit = false;
        tri2Hit = false;
        tri3Hit=false;
        sphere1Hit = false;
        sphere2Hit = false;
        eye = Point3D(0, 0, -1800);                //camera/eye origin
        MAX_DEPTH = 3;
        GLOBAL_AMBIENT = 0.3;
        bgColor = QColor(0, 0, 0, 0);              //background color (black)

        i = 0;          //counter of intersection hits for debugging
    }

    Vector3D sphereNormal(Point3D p, int sphereID){
        Vector3D ret = p - objects[sphereID]->center;
        ret.normalize();
        return ret;
    }

    Vector3D reflect(Vector3D d, Vector3D n){
        return d-2*d.dot(n) * n;
    }

    //casting the ray
    //trace based off of course notes: https://d2l.ucalgary.ca/d2l/le/content/106725/viewContent/1803013/View
    QColor trace(Point3D p, Vector3D d, int depth){
        QColor local, reflected;
        Point3D q;          //intersection point
        Vector3D n, r;   //normal reflection transmission

        if(depth > MAX_DEPTH){
            return  bgColor;
        }

        //intersect for sphere ONE ID# 0
        q = intersect(p, d);

        //get sphere normal and reflect and transmit
        if(sphere1Hit == true){
            n = sphereNormal(q, 0);

            q = q + (0.1*n); //correction to intersection point 'q'

            r = reflect(d, n);
            r.normalize();
            //passing sphere material
            local = phong(q, n, objects[0]->mat, 0);
            reflected = trace(q, r, depth+1);
        }

        //intersect for sphere TWO ID# 1
        q = intersect(p, d);

        //get sphere normal and reflect and transmit
        if(sphere2Hit == true){
            n = sphereNormal(q, 1);

            q = q + (0.1*n); //correction to intersection point 'q'

            r = reflect(d, n);
            r.normalize();
            //passing sphere material
            local = phong(q, n, objects[1]->mat, 1);
            reflected = trace(q, r, depth+1);
        }


        //check for a ray hit on triangle
        triIntersect(p, d, 2);

        if(tri1Hit == true){
            r = reflect(d, objects[2]->triNorm);
            r.normalize();
            local = phong(q, objects[2]->triNorm, objects[2]->mat, 2);
            reflected = trace(q, r, depth+1);
        }

        //check for ray hit on one of the quad's triangles
        triIntersect(p, d, 3);

        if(tri2Hit == true){
            r = reflect(d, objects[3]->triNorm);
            r.normalize();
            local = phong(q, objects[3]->triNorm, objects[3]->mat, 3);
            reflected = trace(q, r, depth+1);
        }

        //check for ray hit on the other Triangle that makes up the quad
        triIntersect(p, d, 4);

        if(tri3Hit == true){
            r = reflect(d, objects[4]->triNorm);
            r.normalize();
            local = phong(q, objects[4]->triNorm, objects[4]->mat, 4);
            reflected = trace(q, r, depth+1);
        }

        double R = local.red() + reflected.red();
        double G = local.green() + reflected.green();
        double B = local.blue() + reflected.blue();

        R = clamp(R);
        G = clamp(G);
        B = clamp(B);

        QColor color = QColor(R, G, B, 0);
        return color;
    }

    double clamp(double c){
        if(c > 255){
            return 255;
        }
        if(c<0){
            return 0;
        }
        return c;
    }

    //phong based off of course notes: https://d2l.ucalgary.ca/d2l/le/content/106725/viewContent/1803013/View
    QColor phong(Point3D p, Vector3D n, Material * material, int objectID){
        double ret = material->ka * GLOBAL_AMBIENT;

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

            //sphere 2
            if(objectID == 1){
                if(sphere2Hit == false){
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
        }

        ret = clamp(ret);

        QColor col;
        //if a sphere's being shaded, shade it this color
        if(objectID == 0){
            col = QColor(0,ret,ret, 0);
        }
        //if our second sphere is being shaded
        else if(objectID == 1){
            col = QColor(ret, ret, 30, 0);
        }
        //if our is tri being shaded, shade it this color
        else if(objectID == 2){
            col = QColor(ret, 70, ret, 0);
        }
        //else it's our quad, shade quad this color
        else if(objectID >2){
            col = QColor(ret, ret, ret, 0);
        }

        return col;
    }

    //triangle intersections from:
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    void triIntersect(Point3D p, Vector3D d, int triID){
        //CHECKING FOR TRIANGLE INTERSECTIONS
        Vector3D AB = objects[triID]->v2 - objects[triID]->v1;
        Vector3D AC = objects[triID]->v3 - objects[triID]->v1;
        Vector3D BC = objects[triID]->v3 - objects[triID]->v2;

        Vector3D triNormal = AB.cross(AC);
        triNormal.normalize();
        objects[triID]->triNorm = triNormal;

        float det, inv_det, u, v, t;

        Vector3D P = d.cross(AC);
        det = AB.dot(P);

        if(det==0){
            if(triID == 2){
                tri1Hit = false;
            }
            if(triID == 3){
                tri2Hit = false;
            }
            if(triID == 4){
                tri3Hit = false;
            }
            return;
        }
        inv_det = 1.0f/det;

        Vector3D T = p - objects[triID]->v1;
        u =T.dot(P) * inv_det;

        if(u < 0.0f || u>1.0f){
            if(triID == 2){
                tri1Hit = false;
            }
            if(triID == 3){
                tri2Hit = false;
            }
            if(triID == 4){
                tri3Hit = false;
            }
            return;
        }

        Vector3D Q = T.cross(AB);

        v= d.dot(Q) * inv_det;

        if(v < 0.0f || (u + v)>1.0f){
            if(triID == 2){
                tri1Hit = false;
            }
            if(triID == 3){
                tri2Hit = false;
            }
            if(triID == 4){
                tri3Hit = false;
            }
            return;
        }

        t = AC.dot(Q) * inv_det;

        if(t > 0){
            if(triID == 2){
                tri1Hit = true;
            }
            if(triID == 3){
                tri2Hit = true;
            }
            if(triID == 4){
                tri3Hit = true;
            }
            return;
        }

        if(triID == 2){
            tri1Hit = false;
        }
        if(triID == 3){
            tri2Hit = false;
        }
        if(triID == 4){
            tri3Hit = false;
        }
        return;
    }

    Point3D intersect(Point3D p, Vector3D d){
        //CHECKING FOR SPHERE INTERSECTION
        //following sphere intersect equation from:
        //http://pages.cpsc.ucalgary.ca/~eharris/cpsc453/tut17/

        //sphereID 0 is first sphere inserted, ID 1 is the second sphere inserted

        Vector3D D = p - objects[0]->center;
        double x = qPow(d.dot(D), 2);
        double y = D.dot(D) - qPow(objects[0]->radius, 2);
        double z = x - y;
        double sqroot = qSqrt(z);
        double b = -(d.dot(D));

        double t;
        double t1;
        double t2;
        double chosenT;
        Point3D interPoint;

        //check discriminant
        if (z < 0){
            sphere1Hit = false;
        }

        else{
            //if discriminant is exactly 0 then only one root, use that root
            if(z == 0){
                t = b + sqroot;
                chosenT = t;
                interPoint = p +t*d;
                sphere1Hit = true;
            }

            //if discriminant > 0 take smallest positive root
            if (z > 0){
                t1 = b - sqroot;
                t2 = b + sqroot;

                if(t1<0 && t2 < 0){
                    sphere1Hit = false;
                }

                else{
                    if(t1 < 0){
                        chosenT = t2;
                        interPoint = p + t2*d;

                        /**debugging**/
                        i++; //intersect counter
                        sphere1Hit = true;
                    }
                    else if(t2 < 0){
                        chosenT = t1;
                        interPoint = p + t1*d;

                        /**debugging**/
                        i++; //intersect counter

                        sphere1Hit = true;
                    }
                    else{
                        if(t2 < t1){
                            chosenT = t2;
                            interPoint = p + t2*d;

                            /**debugging**/
                            i++; //intersect counter

                            sphere1Hit = true;
                        }
                        else{
                            interPoint = p + t1*d;
                            chosenT = t1;

                            /**debugging**/
                            i++; //intersect counter
                            sphere1Hit = true;
                        }
                    }
                }
            }
        }
//**********************CHECKING SECOND SPHERE INTERSECTION*****************************************
        D = p - objects[1]->center;
        x = qPow(d.dot(D), 2);
        y = D.dot(D) - qPow(objects[1]->radius, 2);
        double z2 = x - y;
        sqroot = qSqrt(z2);
        b = -(d.dot(D));

        double s2t;
        double s2t1;
        double s2t2;
        double chosenT2;

        //check discriminant
        if (z2 < 0){
            sphere2Hit = false;
            return interPoint;
        }

        else{
            //if discriminant is exactly 0 then only one root, use that root
            if(z2 == 0){
                s2t = b + sqroot;
                interPoint = p +s2t*d;
                chosenT2=s2t;
                sphere2Hit = true;
            }

            //if discriminant > 0 take smallest positive root
            if (z2 > 0){
                s2t1 = b - sqroot;
                s2t2 = b + sqroot;

                if(s2t1<0 && s2t2 < 0){
                    sphere2Hit = false;
                }

                else{
                    if(s2t1 < 0){
                        interPoint = p + s2t2*d;
                        chosenT2=s2t2;

                        ///**debugging
                        i++; //intersect counter
                        sphere2Hit = true;
                    }
                    else if(s2t2 < 0){
                        interPoint = p + s2t1*d;
                        chosenT2=s2t1;

                        //**debugging
                        i++; //intersect counter

                        sphere2Hit = true;
                    }
                    else{
                        if(s2t2 < s2t1){
                            interPoint = p + s2t2*d;
                            chosenT2=s2t2;

                            //**debugging
                            i++; //intersect counter

                            sphere2Hit = true;
                        }
                        else{
                            interPoint = p + s2t1*d;
                            chosenT2=s2t1;

                            //**debugging
                            i++; //intersect counter
                            sphere2Hit = true;
                        }
                    }
                }
            }
        }

        //choosing which sphere's intersect point to use and draw
        Point3D temp1 = p+ chosenT*d;
        Vector3D temp2 = p-temp1;
        double length = temp2.length();

        Point3D temp3 = p+chosenT2*d;
        Vector3D temp4 = p - temp3;
        double length2 = temp4.length();

        if(z >=0 && z2 >=0){
            if(length < length2){
                interPoint = p+chosenT*d;
                //sphere1Hit = true;
                //sphere2Hit = false;
            }
            else{
                interPoint = p+chosenT2*d;
                //sphere1Hit = false;
                //sphere2Hit = true;
            }
        }

        return interPoint;
    }
};

int main(int argc, char *argv[])
{
    // currently unused parameters
    Q_UNUSED(argc);

    // image width and height
    Scene scene = Scene();

    //set scene size
    scene.width = atoi(argv[1]);
    scene.height = atoi(argv[2]);

    //sphere initialization - sphereID# 0
    Sphere * sphere = new Sphere();
    sphere->center = Point3D(350, 250, 0);
    scene.objects.push_back(sphere);

    //add a second sphere - sphereID# 1
    Sphere*sphere1 = new Sphere();
    sphere1->center = Point3D(150, 250, 10);
    sphere1->radius = 30;
    scene.objects.push_back(sphere1);

    //triangle initialization
    Triangle * triangle = new Triangle();
    scene.objects.push_back(triangle);

    //quad initialization (2 triangles)
    Triangle * half1 = new Triangle();
    half1 -> v1 = Point3D(250,120, 0);
    half1 -> v2 = Point3D(180, 20, 0);
    half1 -> v3 = Point3D(250, 20, 0);
    scene.objects.push_back(half1);

    /*
    half1 -> v1 = Point3D(160, 160, -10);
    half1 -> v2 = Point3D(170, 100, -10);
    half1 -> v3 = Point3D(180, 150, -10);*/

    Triangle * half2 = new Triangle();
    half2 -> v1 = Point3D(180,120,0);
    half2 -> v2 = Point3D(180, 20,0);
    half2 -> v3 = Point3D(250, 120, 0);
    scene.objects.push_back(half2);

    //add a light source
    Light * newLight = new Light();
    scene.lights.push_back(newLight);

    /*
    Light * newLight2 = new Light();
    newLight2->pos =Point3D(-1000, 0, -1000);
    scene.lights.push_back(newLight2);*/

    //depth starting at 0
    int depth = 0;

    //storing these values for ease of use
    //size of output image in pixels
    int width = scene.width;
    int height = scene.height;

    //size of image in world coords (image plane)
    double xMax = width;
    double xMin = 0;
    double yMax = height;
    double yMin = 0;

    //ratios 1:1
    double deltaX = (xMax - xMin)/width;
    double deltaY = (yMax - yMin)/height;

    // create new image
    QImage image(width, height, QImage::Format_RGB32);

/****************** Raytrace Loop********************/
//raytracer based off of http://pages.cpsc.ucalgary.ca/~eharris/cpsc453/tut17/
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
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
/*************************************************************/

    /**debugging**/
    qDebug()<<"Debug > TIMES HIT: " << scene.i;

    // save to file
    QTextStream cout(stdout);
    QString value;
    cout<< "Enter a name for the generated picture: ";
    cout.flush();
    QTextStream cin(stdin);
    value = cin.readLine();

    value.append(".png");

    /**debugging**/
    qDebug()<<"Debug > " << value << " has been created.";

    //TODO: prompt user on command line for output name
    image.save(value);

    // application successfully returned
    return 0;
}


