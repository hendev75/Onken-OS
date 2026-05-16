/*
 * donut.c - A rotating ASCII donut in C, but the code is a donut aswell.
 * Created by Andy Sloane (@a1k0n) in 2006.
 * * Source: https://www.a1k0n.net/2006/09/15/obfuscated-c-donut.html
 *
 * LICENSE NOTE: 
 * This file is included in BoredOS as a demonstration. 
 * This specific file is NOT covered by the project's GPLv3 license 
 * and remains the intellectual property of Andy Sloane. 
 * Use of this file is subject to the author's original terms.
 */
#include <stdlib.h>
#include <syscall.h>

int putchar(int c) {
    char ch = (char)c;
    return sys_write(1, &ch, 1);
}

int k;double sin()
         ,cos();int main(){float A=
       0,B=0,i,j,z[1760];char b[
     1760];printf("\x1b[2J");for(;;
  ){memset(b,32,1760);memset(z,0,7040)
  ;for(j=0;6.28>j;j+=0.07)for(i=0;6.28
 >i;i+=0.02){float c=sin(i),d=cos(j),e=
 sin(A),f=sin(j),g=cos(A),h=d+2,D=1/(c*
 h*e+f*g+5),l=cos      (i),m=cos(B),n=s\
in(B),t=c*h*g-f*        e;int x=40+30*D*
(l*h*m-t*n),y=            12+15*D*(l*h*n
+t*m),o=x+80*y,          N=8*((f*e-c*d*g
 )*m-c*d*e-f*g-l        *d*n);if(22>y&&
 y>0&&x>0&&80>x&&D>z[o]){z[o]=D;;;b[o]=
 ".,-~:;=!*#$@"[N>0?N:0];}}/*#****!!-*/
  printf("\x1b[H");for(k=0;1761>k;k++)
   putchar(k%80?b[k]:10);A+=0.04;B+=
     0.02;}}/*****####*******!!=;:~
       ~::==!!!**********!!!==::-
         .,~~;;;========;;;:~-.
             ..,--------,*/