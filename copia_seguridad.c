#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define L 16
#define V L*L
#define DEBUG
#define TER
#define N_Datos_Max 1000


char leer_flag(void);
void Genera_configuracion_Inicial(char *s); //Puede generarla o leerla
void lee_input (double *beta_0, double *beta_f, double *dbeta, int *N_Ter, int *N_med, int *N_met, int *semillarapuano); //Lee beta_inicial, beta_final,delta_beta,N_Ter,N_med,N_Met y la bandera para el tipo de semilla, si quereis lo puedo cambiar de sitio
void inicializo_direcciones(int *xp, int *yp, int *xm, int *ym); //inicializo los vectores de movimiento
void metropolis(int[], int[], int[], int[], char[], double[]); //Actualiza los spines de una configuracion y decide si acepta o no el cambio
void calcular_prob(double[], double); //Necesario para optimizar la funcion metropolis()
void med_error(double *datos, int N_datos, double *media, double *varianza);//calcula la media y el error
double energia(char[], int[], int[]);
double magnetizacion(char[]);
double cs_error(int n_bloques, int n_datblo, double *vnorm);
double em_error(int n_bloques, int n_datblo, double *vnorm);
void escribe(char s[],FILE *fdebug);


///Parisi-Rupano
#define NormRANu (2.3283063671E-10F)
unsigned int irr[256];
unsigned int ir1;
unsigned char ind_ran,ig1,ig2,ig3;
extern float random(void);
extern void ini_ran(int SEMILLA);


int main() {
  double beta, beta_inicial, beta_final, delta_beta, e,ee, emedia, m, mmedia , em, e2, m2, cv, ecv, sx, esx, aux;//e es energia, m magnetizacion, ee es el error de la energia, em es el error de la magneticacion y e2 y m2 es el cuadrado, cv calor, sx suceptibilidad y los errores
  double prob[5], eres[N_Datos_Max], mres[N_Datos_Max], mresabsoluto[N_Datos_Max],eres2[N_Datos_Max], mres2[N_Datos_Max];
  int N_Ter, N_med, N_Met, N_pasos, tipo_semilla, N_betas, N_m, N_M, j, N_datblo, N_bloques;
  int xp[L], xm[L], yp[L], ym[L];
  char s[V];

  FILE*fdebug;  //se utiliza en la funcion escribe(), que saca un archivo para mirar una configuracion con gnuplot
  fdebug=fopen("conf_debug.dat","wb");
  FILE*fout;    //creo el archivo de resultados
  fout=fopen("datos.txt","wt"); //esto sirve para reiniciar el archivo de resultados al usar el programa despu�s de haberlo usado
  fclose(fout);
  fout=fopen("datos.txt","at"); //abro el archivo en el que se van a escribir los resultados, es de tipo append
  #ifdef TER
  FILE *fter=fopen("ter.txt","wt"); //en este archivo representare magnetizacion y energia frente al numero de medidas para termalizar bien
  #endif // TER
  inicializo_direcciones(xp, yp, xm, ym);
  lee_input(&beta_inicial, &beta_final, &delta_beta, &N_Ter, &N_med, &N_Met, &tipo_semilla);
  ini_ran(123456789); //la semilla hay que cambiarla en algun punto, pero como no s� poner el tiempo del sistema pues aun no tengo la funcion

  Genera_configuracion_Inicial(s);
  #ifdef DEBUG
  escribe(s,fdebug);
  #endif // DEBUG

  /// HASTA AQU� FUNCIONA BIEN

  N_pasos=(beta_final-beta_inicial)/delta_beta;//atencion: N_pasos es entero
  beta=beta_inicial;
  N_datblo=50;
  N_bloques=N_med/N_datblo;
  for(int sentido=0;sentido<1;sentido++)
  {
    for(N_betas=0;N_betas<=N_pasos+1;N_betas++)
    {
      calcular_prob(prob,beta); //Calculamos la tabla de probabilidad para la nueva beta
      for(N_m=0;N_m<N_Ter;N_m++) metropolis(xp,yp,xm,ym,s,prob); //Proceso de termallizacion
      for(N_m=0;N_m<N_med;N_m++) //bucle en medidas
      {
        for(N_M=0;N_M<N_Met;N_M++) metropolis(xp,yp,xm,ym,s,prob); //Iteraciones de Monte Carlo
        eres[N_m]=energia(s,xp,yp);//Guarda resultados en un cada vector correspondiente
        mres[N_m]=magnetizacion(s);
        mresabsoluto[N_m]=fabs(mres[N_m]);//lo que se busca es la media del valor absoluto de m
        #ifdef TER
        fprintf(fter,"%i\t%lf\t%lf\n",N_m,eres[N_m],mres[N_m]);
        #endif // TER
      }
      med_error(eres,N_med,&emedia,&ee);//hago la media y el error de los resultados
      med_error(mresabsoluto,N_med,&mmedia,&em);
      for(j=0;j<N_med;j++){//elevo cada termino de e y m al cuadrado para calcular la media de e^2 y m^2
                eres2[j]=eres[j]*eres[j];
                mres2[j]=mres[j]*mres[j];
      }
      med_error(eres2,N_med,&e2,&aux);//calculo la media de e^2 y m^2, como no necesito el error de estos resultados uso el auxiliar aux
      med_error(mres2,N_med,&m2,&aux);
      cv=V*(e2-emedia*emedia); //calculo del calor especifico, y abajo su error
      ecv=cs_error(N_datblo,N_bloques,eres); // LOS DOS PRIMEROS VALORES DE AQU� LOS HE PUESTO POR PONER
      sx=V*(m2-mmedia*mmedia); //calculo de x (chi), y abajo su error
      esx=cs_error(N_datblo,N_bloques,mres);
      fprintf(fout,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",beta,emedia,ee,mmedia,em,e2,m2,cv,ecv,sx,esx);//escribo los resultados
      //Escribe_Informacion_en_Pantalla(); // VA todo bien??
      beta+=delta_beta; //Incremento el valor de beta
      }
    delta_beta=-delta_beta;
   }
  fclose(fout);
  return 0;
 }

void metropolis(int xp[], int yp[], int xm[], int ym[], char s[], double prob[]){
    int indice, n=0, x, y;
    for (y=0; y<L; y++){
        for (x=0; x<L; x++){
            /* Calculamos el indice del vectro prob[] */
            indice=s[n]*(s[n+xp[x]]+s[n+yp[y]]+s[n+xm[x]]+s[n+ym[y]])/2+2;
            if (random()<prob[indice]) s[n]=-s[n]; //Aceptamos o no el cambio de spin
            n++;
        }
    }
}

void calcular_prob (double prob[], double beta){
    /* Calculamos, para cada beta, los posibles valores
       de la diferencia de energia al cambiar un spin,
       esta funcion se implementa en la funcion metropolis()*/

    prob[0]=exp(8*beta);
    prob[1]=exp(4*beta);
    prob[2]=1;
    prob[3]=exp(-4*beta);
    prob[4]=exp(-8*beta);
}

char leer_flag(void){ //lee el tipo de configuracion que queremos tener
    char x;
    FILE *flag;
    if ((flag=fopen("bandera.txt","rt"))==NULL){
        printf("el archivo que lee bandera no existe o no se puede abrir");
        exit(2);
    }
    else {
        fscanf(flag,"%c",&x); //revisar si he hecho bien en pasar puntero
        fclose(flag);
    }
    return x;
}

void Genera_configuracion_Inicial(char *s){ //crea la configuracion inicial
    int i,j;
    int bandera;
    bandera=leer_flag();
    FILE *fconfig;
    switch(bandera){
    case '0': // crea una distribucion random
        for(i=0;i<V;i++){
            if(random()<0.5)
                s[i]=1;
            else
                s[i]=-1;
        }
        break;
    case '1': //crea una distribucion uniforme (todo -1 o 1)
        if(random()<0.5){
            for(i=0;i<V;i++)
                s[i]=1;
        }
        else{
            for(i=0;i<V;i++)
                s[i]=-1;
        }
        break;
    case '2': // lee la configuracion de un ficheiro de texto �deber�a ser binario?
        if((fconfig=fopen("configuracion.txt","rt"))==NULL){
            printf("No existe o no se puede abrir el archivo de la configuracion inicial");
            exit(3);
        }
        else {
            for(i=0;i<V;i++)
                fscanf(fconfig,"%c",&s[i]);
        }
        fclose(fconfig);
        break;
    case '3': // esta ser� la que la crea en ajedrez
        for(j=0;j<L;j++)
            for(i=0;i<L;i++){
                s[i+(L*j)]=pow(-1,i+j);
            }
        break;
    }
}

void lee_input (double *beta_0, double *beta_f, double *dbeta, int *N_Ter, int *N_med, int *N_met, int *semillarapuano){ // se introducen los parametros del sistema adem�s de una bandera para saber c�mo elijo la semilla
    FILE *f_in;
    if ((f_in=fopen("input.txt","rt"))==NULL){
        printf("el archivo de input no existe o no se puede abrir");
        exit(1);
    }
    else{
        fscanf(f_in,"%lf %lf %lf %d %d %d %d",beta_0,beta_f,dbeta,N_Ter, N_med, N_met, semillarapuano); // de nuevo revisar si hice bien en pasar punteros (dir�a que s�)
    }
}

void inicializo_direcciones(int *xp, int *yp, int *xm, int *ym){
    int i;
    for (i=0;i<(L-1);i++)
        xp[i]=1;
    xp[L-1]=-(L-1);
    for (i=0;i<(L-1);i++)
        yp[i]=L;
    yp[L-1]=-L*(L-1);
    for(i=1;i<L;i++)
        xm[i]=-1;
    xm[0]=(L-1);
    for(i=1;i<L;i++)
        ym[i]=-L;
    ym[0]=L*(L-1);
}

double energia(char s[], int xp[], int yp[]){
    double u=0;
    int n=0;
    for (int j=0;j<L;j++){
         for (int i=0;i<L;i++)  {
            u+=(double)s[n]*(s[n+xp[i]]+s[n+yp[j]]);
            n++;
        }
    }
    return -u/(2*L*L);
}

double magnetizacion(char s[]){
    double m=0;
    for (int i=0;i<L*L;i++)     m+=s[i]; //Sumamos los spines
    return m/(L*L);
}

void ini_ran(int SEMILLA){ //Parisi-Rupano, NO TOCAR
    int INI,FACTOR,SUM,i;
    srand(SEMILLA);
    INI=SEMILLA;
    FACTOR=67397;
    SUM=7364893;
    for(i=0;i<256;i++) {
        INI=(INI*FACTOR+SUM);
        irr[i]=INI;
    }
    ind_ran=ig1=ig2=ig3=0;
}

float random(void){ //N� aleatorio entre [0,1)
    float r;
    ig1=ind_ran-24;
    ig2=ind_ran-55;
    ig3=ind_ran-61;
    irr[ind_ran]=irr[ig1]+irr[ig2];
    ir1=(irr[ind_ran]^irr[ig3]);
    ind_ran++;
    r=ir1*NormRANu;
    return r;
}

void med_error(double *datos, int N_datos, double *media, double *e){
    double sumx=0,sumx2=0;
    int i;
    for(i=0;i<N_datos;i++){
        sumx=sumx+datos[i];
        sumx2=sumx2+(pow(datos[i],2));
    }
    *media=sumx/N_datos;
    sumx2=sumx2/N_datos;
    sumx=pow(*media,2);
    *e=sqrt((sumx2-sumx)/(N_datos-1));
}

double cs_error(int n_bloques, int n_datblo, double *vnorm)              //ESTIMACION DEL ERROR DE CV O X (NO DEL VALOR)
{                                                                        //n_bloques*n_datblo <= N_m (idealmente igual), vnorm: vector de e o m para un beta
    int ics, jcs;                                                        //Divide vnorm en n_bloques de n_datblo valores de e o m, y calcula el CV o X para cada bloque
    double sumb=0,sumb2=0, gb[n_bloques], sumg=0, sumg2=0;               //Con la desviaci�n t�pica de estos CV o X estima el error, pero el valor real se calcula sin
    for(ics=0;ics<n_bloques;ics++)                                       //an�lisis por bloques.
    {
        for(jcs=0;jcs<n_datblo;jcs++)
        {
            sumb+=vnorm[n_datblo*ics+jcs];
            sumb2+=vnorm[n_datblo*ics+jcs]*vnorm[n_datblo*ics+jcs];
        }
        gb[ics]=V*(sumb2/n_datblo-sumb*sumb/(n_datblo*n_datblo));
        sumb=0;
        sumb2=0;
        sumg+=gb[ics];
        sumg2+=gb[ics]*gb[ics];
    }
    return sqrt((sumg2/n_bloques-sumg*sumg/(n_bloques*n_bloques))/n_bloques);
}

double em_error(int n_bloques, int n_datblo, double *vnorm)              //ESTIMACION DEL ERROR DE E O |M| (NO DEL VALOR)
{                                                                        //Entradas: las mismas que el anterior. Calcula las medias de los bloques y estima el error
    int iem, jem;                                                        //con la desviaci�n t�pica entre las medias (an�lisis por bloques). Tendremos que ver
    double sumb=0, emb[n_bloques], sumem=0, sumem2=0;                   //para que n_datblo (tanto en esta como en la anterior) el error se estabiliza.
    for(iem=0;iem<n_bloques;iem++)                                       //Probablemente se puedan fusionar las dos funciones pero lo he hecho separado de primeras para aclararme.
    {
        for(jem=0;jem<n_datblo;jem++)
        {
            sumb+=vnorm[n_datblo*iem+jem];
        }
        emb[iem]=sumb/n_datblo;
        sumb=0;
        sumem+=emb[iem];
        sumem2+=emb[iem]*emb[iem];
    }
    return sqrt((sumem2/n_bloques-sumem*sumem/(n_bloques*n_bloques))/n_bloques);
}

void escribe(char s[],FILE *fdebug){
     for(int i=0;i<L*L;i++) 	fprintf(fdebug,"%d%c",s[i],(i+1)%L==0?'\n':' ');
}
