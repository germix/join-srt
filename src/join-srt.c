////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Programa para unir archivo de subtítulos (SRT) cortados
//
// Este es la remake de un programa viejo hecho en el 2010
// el cual hice para unir subtítulos de una película que encontré que
// tenía los subtítulos separados y no sabía como unirlos
//
// Germán Martínez - 13/03/2017
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

// Tamaño del buffer de línea
#define MAX_LINE_SIZE					0x1000

// Strings (English, Español)
#if 1
#define STR_USAGE					"Usage"
#define STR_CANNOT_OPEN_DST_FILE	"Can't open destination file \"%s\""
#define STR_CANNOT_OPEN_SRC_FILE	"Can't open source file \"%s\""
#define STR_CANNOT_READ_SRC_FILE	"Can't read from source file \"%s\""
#define STR_CANNOT_WRITE_DST_FILE	"Can't write to destination file \"%s\""
#else
#define STR_USAGE					"Usar"
#define STR_CANNOT_OPEN_DST_FILE	"No se puede abrir el archivo de destino \"%s\""
#define STR_CANNOT_OPEN_SRC_FILE	"No se puede abrir el archivo de origen \"%s\""
#define STR_CANNOT_READ_SRC_FILE	"No se puede leer desde el archivo de origen \"%s\""
#define STR_CANNOT_WRITE_DST_FILE	"No se puede escribir en el archivo de destino \"%s\""
#endif

#ifndef __cplusplus
typedef char bool;
#define true 1
#define false 0
#endif

struct time
{
	int h;			// Hora
	int m;			// Minutos
	int s;			// Segundos
	int ms;			// Milisegundos
};

enum
{
	SECS_PER_DAY = 86400,			// Segundos por día
	MSECS_PER_DAY = 86400000,		// Milisegundos por día
	SECS_PER_HOUR = 3600,			// Segundos por hora
	MSECS_PER_HOUR = 3600000,		// Milisegundos por hora
	SECS_PER_MIN = 60,				// Segundos por minuto
	MSECS_PER_MIN = 60000,			// Milisegundos por minuto
};

static unsigned int time_to_uint(const struct time* t)
{
	return (t->h*SECS_PER_HOUR + t->m*SECS_PER_MIN + t->s)*1000 + t->ms;
}
static void uint_to_time(unsigned int u, struct time* t)
{
	t->h	= (u / MSECS_PER_HOUR);
	t->m	= (u % MSECS_PER_HOUR) / MSECS_PER_MIN;
	t->s	= (u / 1000) % SECS_PER_MIN;
	t->ms	= (u % 1000);
}
//
// Leer el tiempo
//
const char* read_time(const char* p, struct time* t)
{
	char tmp[5];
	
	// Leer hora
	tmp[0] = *p++;
	tmp[1] = *p++;
	tmp[2] = '\0';
	t->h = atoi(tmp);
	p++; // saltar ':'
	
	// Leer minutos
	tmp[0] = *p++;
	tmp[1] = *p++;
	tmp[2] = '\0';
	t->m = atoi(tmp);
	p++; // saltar ':'
	
	// Leer segundos
	tmp[0] = *p++;
	tmp[1] = *p++;
	tmp[2] = '\0';
	t->s = atoi(tmp);
	p++; // saltar ','
	
	// Leer milisegundos
	tmp[0] = *p++;
	tmp[1] = *p++;
	tmp[2] = *p++;
	tmp[3] = '\0';
	t->ms = atoi(tmp);

	return p;
}
//
// Imprimir tiempo: inicial --> final
//
void print_times(FILE* fp, unsigned int base, const struct time* show, const struct time* hide)
{
	struct time f_show;
	struct time f_hide;
	
	uint_to_time(base+time_to_uint(show), &f_show);
	uint_to_time(base+time_to_uint(hide), &f_hide);
	
	fprintf(fp, "%02d:%02d:%02d,%003d --> %02d:%02d:%02d,%003d\n",
		f_show.h,
		f_show.m,
		f_show.s,
		f_show.ms,
		f_hide.h,
		f_hide.m,
		f_hide.s,
		f_hide.ms);
}
//
// Main
//
int main(int argc, char* argv[])
{
	if(argc >= 3)
	{
		int i;
		FILE* in;
		FILE* out;
		const char* p;
		static char line[MAX_LINE_SIZE];
		struct time time_show;
		struct time time_hide;
		unsigned int sub_num;
		unsigned int last_time;
		
		//
		// Crear el archivo de destino
		//
		if(NULL == (out = fopen(argv[1], "w")))
		{
			fprintf(stderr, STR_CANNOT_OPEN_DST_FILE, argv[1]);
			return -1;
		}
		//
		// Iterar sobre cada uno de los archivo de origen y escribirlos en el archivo de destino
		//
		sub_num = 0;
		last_time = 0;
		for(i = 2; i < argc; i++)
		{
			// Abrir archivo de origen
			if(NULL == (in = fopen(argv[i], "r")))
			{
				fclose(out);
				fprintf(stderr, STR_CANNOT_OPEN_SRC_FILE, argv[1]);
				return -1;
			}
			//
			// Leer y escribir
			//
			while(!feof(in))
			{
				//
				// Leer número
				//
				line[0] = 0;
				fgets(line, MAX_LINE_SIZE, in);
				if(line[0] < 0)
					continue;
				if(!isdigit(line[0]))
					continue;
				
				sub_num++;
				fprintf(out, "%d\n", sub_num);
				
				//
				// Leer tiempo
				//
				{
					p = line;

					// Obtener la línea
					fgets(line, MAX_LINE_SIZE, in);
					
					// Leer el tiempo de cuando se muestra el subtítulo
					p = read_time(p, &time_show);
					
					p++;	// ' '
					p++;	// '-'
					p++;	// '-'
					p++;	// '>'
					p++;	// ' '
					
					// Leer el tiempo de cuando se oculta el subtítulo
					p = read_time(p, &time_hide);
					
					print_times(out, last_time, &time_show, &time_hide);
				}
				//
				// Leer el texto del subttítulo
				//
				while(!feof(in))
				{
					line[0] = 0;
					fgets(line, MAX_LINE_SIZE, in);
					
					fprintf(out, "%s", line);
					if(line[0] == '\n' || line[0] == '\0')
					{
						break;
					}
				}
			}
			last_time += time_to_uint(&time_show);
			//last_time += time_to_uint(&time_hide);
			
			// Cerrar el archivo actual de origen
			fclose(in);
		}
		// Cerra el archivo de destino
		fclose(out);
	}
	else
	{
		printf(STR_USAGE);
		printf(": join-srt <dest> <src1> ... <srcN>");
	}
	return 0;
}

