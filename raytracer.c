#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int width;
int height;

typedef struct
{
        float x,y,z;
} vec3;

typedef struct
{
        int r,g,b;      
} pixel;

typedef struct
{
        vec3 e;
        vec3 d;
} ray;

typedef struct
{
        int r,g,b;      
} color;

typedef struct
{
        vec3 center;
        color sColor;
        float radius;
} sphere;

vec3 eye;
vec3 gaze;
vec3 up;

vec3 u,v,w;

int numLights;
vec3* lightPositions;
color* lightColors;

float d;
float left,right,top,bottom;

sphere* sceneSpheres;
int numSpheres;

char outputFileName[80];

pixel** image;

float dot(vec3 a, vec3 b)
{
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

vec3 cross(vec3 a, vec3 b)
{
    vec3 tmp;
    tmp.x = a.y*b.z-a.z*b.y;
    tmp.y = a.z*b.x-a.x*b.z;
    tmp.z = a.x*b.y-a.y*b.x;
    
    return tmp;
}

float length(vec3 a)
{
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

vec3 normalize(vec3 a)
{
    vec3 tmp;
    float l;
    l = length(a);
    tmp.x = a.x/l;
    tmp.y = a.y/l;
    tmp.z = a.z/l;
    
    return tmp;
}

vec3 add(vec3 a, vec3 b)
{
     vec3 tmp;
     tmp.x = a.x+b.x;
     tmp.y = a.y+b.y;
     tmp.z = a.z+b.z;
     
     return tmp;
}

vec3 multScalar(vec3 v, float s)
{
     vec3 tmp;
     tmp.x = v.x *s;
     tmp.y = v.y *s;
     tmp.z = v.z *s;
     
     return tmp;
}

float intersectSphere(ray r, sphere s)
{
	float A,B,C; //constants for the quadratic equation
	
	float delta;
	
	vec3 scenter;
	
	scenter = s.center;
	
	float t,t1,t2;
	
	C = (r.e.x-scenter.x)*(r.e.x-scenter.x)+(r.e.y-scenter.y)*(r.e.y-scenter.y)+(r.e.z-scenter.z)*(r.e.z-scenter.z)-s.radius*s.radius;

	B = 2*r.d.x*(r.e.x-scenter.x)+2*r.d.y*(r.e.y-scenter.y)+2*r.d.z*(r.e.z-scenter.z);
	
	A = r.d.x*r.d.x+r.d.y*r.d.y+r.d.z*r.d.z;
	
	delta = B*B-4*A*C;
	
	if (delta<0) return -1;
	else if (delta==0)
	{
		t = -B / (2*A);
	}
	else
	{
		delta = sqrt(delta);
		A = 2*A;
		t1 = (-B + delta) / A;
		t2 = (-B - delta) / A;
				
		if (t1<t2) t=t1; else t=t2;
	}
	
	return t;
}
     
void initImage()
{
     int i,j;
     image = (pixel **)malloc(sizeof(pixel*)*height);
     for (i=0;i<height;i++)
         image[i] = (pixel *)malloc(sizeof(pixel)*width);

     for (i=0;i<height;i++)
         for (j=0;j<width;j++)
         {
             image[i][j].r = image[i][j].b = image[i][j].g = 0;
         }
}

void writeImage(char *fileName)
{
     FILE *outfile;
     int i,j;
     char cmd[200];
          
     outfile = fopen(fileName,"w");
     
     if (!outfile)
     {
                  printf("unable to open!\n");
                  return;
     }
     
     fprintf(outfile,"P3\n");
     fprintf(outfile,"%d %d\n",width,height);
     fprintf(outfile,"255\n");
     for (i=0;i<height;i++)
     {
         for (j=0;j<width;j++)
         {
             fprintf(outfile,"%d %d %d ",image[i][j].r,image[i][j].g,image[i][j].b);
         }
         fprintf(outfile,"\n");
     }
     fclose(outfile);
     
     sprintf(cmd,"convert %s %s.jpg",fileName,fileName);
     system(cmd);    
}

readScene(char *fName)
{
 char line[200];
 int lineCnt = 0;
 int sphereCnt = 0;
 int lightCnt = 0;
 
 FILE *sceneFile;
 sceneFile = fopen(fName,"r");

 /* read output file name */ 
 while (fgets(line,200,sceneFile)!=NULL)
 {
   if (line[0]=='#') continue; /* skip the comment line */
   
   lineCnt++;
   switch (lineCnt)
   {
    case 1: printf("reading file name...\n"); sscanf(line,"%s",outputFileName); break;
	case 2: printf("reading width and height...\n"); sscanf(line,"%d %d",&width,&height); break;
	case 3: printf("reading eye position...\n"); sscanf(line,"%f %f %f",&(eye.x),&(eye.y),&(eye.z)); break;
	case 4: printf("reading gaze direction...\n"); sscanf(line,"%f %f %f",&(gaze.x),&(gaze.y),&(gaze.z)); break;
	case 5: printf("reading up direction...\n"); sscanf(line,"%f %f %f",&(up.x),&(up.y),&(up.z)); break;
	case 6: printf("reading d...\n");sscanf(line,"%f",&d); break;
	case 7: printf("reading left, right, top, and bottom...\n"); sscanf(line,"%f %f %f %f",&left,&right,&top,&bottom); break;
	case 8: printf("reading spheres...\n"); sscanf(line,"%d",&numSpheres);
           sceneSpheres = (sphere *)malloc(sizeof(sphere)*numSpheres); 
	   while (sphereCnt<numSpheres)
	   {
 		fgets(line,200,sceneFile);
		if (line[0]=='#') continue;
		sscanf(line,"%f %f %f %f %d %d %d",&(sceneSpheres[sphereCnt].center.x),&(sceneSpheres[sphereCnt].center.y),&(sceneSpheres[sphereCnt].center.z),&(sceneSpheres[sphereCnt].radius),&(sceneSpheres[sphereCnt].sColor.r),&(sceneSpheres[sphereCnt].sColor.g),&(sceneSpheres[sphereCnt].sColor.b));
		sphereCnt++;
        }
	   break;
	case 9: printf("reading lights...\n"); sscanf(line,"%d",&numLights);
           lightPositions = (vec3 *)malloc(sizeof(vec3)*numLights); 
           lightColors = (color *)malloc(sizeof(color)*numLights); 
	   while (lightCnt<numLights)
	   {
 		fgets(line,200,sceneFile);
		if (line[0]=='#') continue;
		sscanf(line,"%f %f %f %d %d %d",&(lightPositions[lightCnt].x),&(lightPositions[lightCnt].y),&(lightPositions[lightCnt].z),&(lightColors[lightCnt].r),&(lightColors[lightCnt].g),&(lightColors[lightCnt].b));
		lightCnt++;
	   }
	   break;
	default: break;/* do nothing */ 
   }
  }

printf("%d %d %d\n",lightColors[0].r,lightColors[0].g,lightColors[0].b);

    v = normalize(up);
    w = multScalar(gaze,-1);
    w = normalize(w);
    u = cross(v,w);
    
}

ray generateRay(int i, int j)
{
    ray tmp;
    float su,sv;
    
    tmp.e = eye;
    
    su = left + ((right-left)*(i+0.5)/width);
    sv = bottom + ((top-bottom)*(j+0.5)/height);
    
    tmp.d = multScalar(u,su);
    tmp.d = add(tmp.d,multScalar(v,sv));
    tmp.d = add(tmp.d,multScalar(gaze,d));
    
    return tmp;
}

color computeColor(int sid, float t, ray r)
{
      color tmp;
      
      color scolor;
      vec3 sref;
      vec3 L, N, H;
      float cosalpha;
      int shadow,i;
      ray lightRay;
      float tt;
      
      vec3 p;
      p = add(eye,multScalar(r.d,t));
      
      scolor = sceneSpheres[sid].sColor;
      
      sref.x = scolor.r;
      sref.y = scolor.g;
      sref.z = scolor.b;
      
      sref = multScalar(sref,1.0/255.0);
      
      // ambient component
      tmp.r = 30*sref.x;
      tmp.g = 30*sref.y;
      tmp.b = 30*sref.z;
      
      // diffuse component
      
      L = add(lightPositions[0],multScalar(p,-1));
      N = add(p,multScalar(sceneSpheres[sid].center,-1));
      shadow = 0;
      lightRay.e = p;
      lightRay.d = L;
      
      for (i=0;i<numSpheres;i++)
      {
          if (i!=sid)
          {
           tt = intersectSphere(lightRay,sceneSpheres[i]);
           if (tt>0.0 && tt<1.0)
           {
                      shadow = 1;
                      break;
           }
          }
      }
      
      if (!shadow)
      {
      
            // diffuse
            L = normalize(L);
            N = normalize(N);
            cosalpha = dot(L,N);
      
            if (cosalpha>0)
            {
             tmp.r += lightColors[0].r*sref.x*cosalpha;
             tmp.g += lightColors[0].g*sref.y*cosalpha;
             tmp.b += lightColors[0].b*sref.z*cosalpha;
            }
            
            // specular
            H = add(L,multScalar(r.d,-1));
            H = normalize(H);
            cosalpha = dot(H,N);
            if (cosalpha>0)
            {
             tmp.r += lightColors[0].r*sref.x*pow(cosalpha,50);
             tmp.g += lightColors[0].g*sref.y*pow(cosalpha,50);
             tmp.b += lightColors[0].b*sref.z*pow(cosalpha,50);
            }
      }
      
      
      
      
      if (tmp.r>255) tmp.r = 255;
      if (tmp.g>255) tmp.g = 255;
      if (tmp.b>255) tmp.b = 255;
      
      return tmp;
      
}
int main(int argc, char **argv)
{
    int i,j,k;
    float minT;
    float t;
    int seenSphere = -1;
    ray r;

    if (argc!=2) { printf("Usage: raytracer <scene file>\n"); return 0; }

    readScene(argv[1]);
    initImage();
    for (i=0;i<width;i++)
        for (j=0;j<height;j++)
        {
            r = generateRay(i,j);
            seenSphere = -1;
            minT = 9000000.0;
            for (k=0;k<numSpheres;k++)
            {
                t = intersectSphere(r,sceneSpheres[k]);
                if (t>1 && t<minT)
                {
                    seenSphere = k;
                    minT = t;
                }
            }
            if (seenSphere!=-1)
            {
                color pixelColor;
                pixelColor = computeColor(seenSphere,minT,r);
                image[height-j-1][i].r = pixelColor.r;
                image[height-j-1][i].g = pixelColor.g;
                image[height-j-1][i].b = pixelColor.b;
            }
        }

    writeImage(outputFileName);
}
