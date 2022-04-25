/* Pract2  RAP 09/10    Javier Ayllon*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> 
#include <assert.h>   
#include <unistd.h>   
#define NIL (0)

#define NUM_WORKERS 8
#define RUTA_IMG "foto.dat"
#define IMG_SIZE 400*400
#define IMG_SIDE 400

/*Variables Globales */

XColor colorX;
Colormap mapacolor;
char cadenaColor[]="#000000";
Display *dpy;
Window w;
GC gc;

/*Funciones auxiliares */

void initX() {

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                     400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);
      for(;;) {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                  break;
      }


      mapacolor = DefaultColormap(dpy, 0);

}

void dibujaPunto(int x,int y, int r, int g, int b) {

        sprintf(cadenaColor,"#%.2X%.2X%.2X",r,g,b);
        XParseColor(dpy, mapacolor, cadenaColor, &colorX);
        XAllocColor(dpy, mapacolor, &colorX);
        XSetForeground(dpy, gc, colorX.pixel);
        XDrawPoint(dpy, w, gc,x,y);
        XFlush(dpy);

}

void receive_pixels(MPI_Comm commPadre){
    int info_pixel[5];
    MPI_Status status;

    for (int i = 0; i < IMG_SIZE; i++){
        MPI_Recv(&info_pixel, 5, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, commPadre, &status);
        dibujaPunto(info_pixel[0],info_pixel[1],info_pixel[2],info_pixel[3],info_pixel[4]);
    }
}

void build_info_pixel(int i, int j, int *rgb, int *info_pixel){
  info_pixel[0] = i;
  info_pixel[1] = j;
  info_pixel[2] = rgb[0];
  info_pixel[3] = rgb[1];
  info_pixel[4] = rgb[2];
}

void read_file(int start, int length, MPI_File handler, MPI_Comm commPadre){
    unsigned char rgb[3];
    int info_pixel[5];
    MPI_Status status;

    for (int i = start; i < (length + start); i++){
      for(int j = 0; j < IMG_SIDE; j++){
        //leer y enviar
        MPI_File_read(handler, rgb, 3, MPI_UNSIGNED_CHAR, &status);

        //build_info_pixel(i, j, rgb, &info_pixel);
        info_pixel[0] = j;
        info_pixel[1] = i;
        info_pixel[2] = rgb[0];
        info_pixel[3] = rgb[1];
        info_pixel[4] = rgb[2];

        MPI_Send(&info_pixel, 5, MPI_INT, 0, 1, commPadre);
      }
    }
}



/* Programa principal */

int main (int argc, char *argv[]) {

  int rank,size;
  MPI_Comm commPadre;
  int tag;
  MPI_Status status;
  int buf[5];

  int errcodes[NUM_WORKERS];


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_get_parent( &commPadre );
  if ( (commPadre==MPI_COMM_NULL) && (rank==0) )  {

	initX();

	/* Codigo del maestro */
    MPI_Comm_spawn("pract2", MPI_ARGV_NULL, NUM_WORKERS, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &commPadre, errcodes);
	
    /*En algun momento dibujamos puntos en la ventana algo como
	dibujaPunto(x,y,r,g,b);  */
    
    receive_pixels(commPadre);

    sleep(10);

  }
  else {
    /* Codigo de todos los trabajadores */
    /* El archivo sobre el que debemos trabajar es foto.dat */
    
    int length = IMG_SIDE/NUM_WORKERS;
    int displacement = (IMG_SIZE*3*sizeof(unsigned char))/NUM_WORKERS;

    MPI_File handler;
    MPI_File_open(MPI_COMM_WORLD, RUTA_IMG, MPI_MODE_RDONLY, MPI_INFO_NULL, &handler);

    MPI_File_set_view(handler, displacement*rank, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, "native", MPI_INFO_NULL);

    read_file(rank*length, length, handler, commPadre);

  }
  MPI_Finalize();

}