#include "hashmap.h"
#include "heap.h"
#include "list.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX 31

typedef struct
{
  char nombre[MAX];
  int prioridad;
  List *precedencia;
  bool visitado;
  bool realizado; // registra si la 
  bool precede;
  bool ingresadoHeap;

} Tarea; // Definimos nuestra estructura a trabajar.

typedef struct
{
  Stack *historial;
  Stack *agregadas;
  Stack *marcadas;
} Pila;

// Subfunciones

void validarOpcion(int *opcion) // Valida las opciones del menú.
{
  char opcionAux[MAX];
  
  while (true) 
  {
    scanf("%30s", opcionAux);
    printf("\n");
      
    if (isdigit(*opcionAux)) 
    {
      *opcion = atoi(opcionAux);
        
      if (*opcion >= 1 && *opcion <= 7) break;
      else printf("Debe ingresar un número válido entre 1 y 7.\n");
    }
    else printf("Debe ingresar un número válido entre 1 y 7.\n");
    
    getchar();
  }
}

void menu() // Opciones del menú.
{
  printf("\nEliga una opción a continuación.\n\n");

  printf("1. Agregar nueva tarea.\n");
  printf("2. Establecer precedencia.\n");
  printf("3. Mostrar tareas pendientes.\n");
  printf("4. Marcar tarea completada.\n");
  printf("5. Deshacer última acción.\n");
  printf("6. Importar tareas.\n");
  printf("7. Salir del programa.\n\n");
}

void validar(int *user_continue) // Validamos que el usuario desee seguir con la ejecución del programa.
{
  char salida[2];
  
  printf("Desea realizar otra operación? (s/n)\n");
  scanf("%1s", salida);
  
  if(strcmp(salida, "n") == 0)
  {
    *user_continue = 0;
    printf("\nGracias por usar el programa, adiós!");
  }
  else while (strcmp(salida, "s") != 0)
  {
    printf("Ingrese una opción válida\n\n");
    printf("Desea realizar otra operación? (s/n)\n");
    scanf("%1s", salida);
  }
}

char *gets_csv_field(char *tmp, int k) //
{
  int open_mark = 0;
  char *ret = (char*) malloc(100 * sizeof(char));
  int ini_i = 0, i = 0;
  int j = 0;
  int last_field = 0;
  
  while (tmp[i + 1] != '\0')
  {  
    if(tmp[i] == '\"')
    {
      open_mark = 1 - open_mark;
      if(open_mark) ini_i = i + 1;
      i++;
      continue;
    }
    
    if(open_mark || tmp[i] != ',')
    {
      if(k == j) ret[i - ini_i] = tmp[i];
      i++;
      continue;
    }

    if(tmp[i] == ',')
    {
      if(k == j) 
      {
        ret[i - ini_i] = 0;
        return ret;
      }
      j++; 
      ini_i = i + 1;
    }
    i++;
  }

  if(k == j) 
  {
    last_field = 1;
    ret[i - ini_i] = 0;
    return ret;
  }

  if (last_field && k == j)
  {
    strcpy(ret + strlen(ret), tmp + ini_i);
    return ret;
  }
  
  return NULL;
}

void preguntarNombreTarea(char *nombreTarea) // Solicita que el usuario ingrese el nombre de la tarea
{
  printf("Ingrese el nombre de la tarea:\n");
  getchar();
  scanf("%30[^\n]s", nombreTarea);
}

void poblarHeap(HashMap *mapaAux, Heap *pendientes)
{
  Pair *aux = firstMap(mapaAux);
  
  while(aux != NULL)
  {
    Tarea *tareaAux = aux -> value;
    
    if(tareaAux -> precede == false && tareaAux -> ingresadoHeap == false) 
    {
      
      heap_push(pendientes, tareaAux -> nombre, tareaAux -> prioridad);
      tareaAux -> ingresadoHeap = true;
    }
    aux = nextMap(mapaAux);
  }
}

// Funciones 
/*
bool tienePrecedencia(Tarea* tarea, List* precedencia) {
  List* aux = firstList(precedencia);
  
  while (aux != NULL) {
    ;
    if (strcmp(tareaActual->nombre, tarea->nombre) == 0)
      return true;
    aux = nextList(precedencia);
  }
  return false;
}
*/
HashMap* copiarMapa(HashMap* mapa)
{
  HashMap* mapaCopia = createMap(10001);

  Pair* aux = firstMap(mapa);
  while (aux != NULL)
  {
    void* clave = aux->key;
    Tarea* valor = aux->value;

    insertMap(mapaCopia, clave, valor);
    aux = nextMap(mapa);
  }

  return mapaCopia;
}
void reiniciarheap(HashMap *mapaAux, Heap *pendientes)
{
  Pair *aux = firstMap(mapaAux);
  
  while(aux != NULL)
  {
    Tarea *tareaAux = aux -> value;
    tareaAux->ingresadoHeap =false;
    aux =nextMap(mapaAux);
  }
}

// 1.
void agregarTarea(HashMap *mapa, Pila *stack, int *registrada)
{
  Tarea *tareaAux = malloc(sizeof(Tarea));
  tareaAux->precedencia = createList();

  char nombreTarea[MAX];
  int prioridad;

  preguntarNombreTarea(nombreTarea);
  
  strcpy(tareaAux -> nombre, nombreTarea);

  printf("\nIngrese la prioridad de la tarea:\n");
  scanf("%d", &prioridad);

  tareaAux -> prioridad = prioridad;
  
  insertMap(mapa, tareaAux -> nombre, tareaAux);
  Pair *par = searchMap(mapa, nombreTarea);

  (*registrada)++;
  tareaAux -> precede = false;
  tareaAux -> realizado = false;
  tareaAux->visitado = false;
  tareaAux -> ingresadoHeap = false;

  stack_push(stack -> historial, "1");
  stack_push(stack -> agregadas, par);
  
  printf("\nTarea agregada!!\n\n");
}

// 2.
void establecerPrecedencia(HashMap *mapa, Pila *stack)
{
  char nombreTarea1[MAX],nombreTarea2[MAX];

  printf("Ingrese el nombre para establecer la precedecia.\n\n(Tarea Precedente): ");
  getchar();
  scanf("%30[^\n]s", nombreTarea1);
  getchar();
  printf("(Tarea Procedente): ");
  scanf("%30[^\n]s", nombreTarea2);

  if(searchMap(mapa, nombreTarea1) == NULL || searchMap(mapa, nombreTarea2) == NULL)
  {
    printf("\nUna o ambas tareas no han sido agregadas aún.\n\n");
    printf("Ingrese nombre de tareas validos");
      getchar();
      scanf("%30[^\n]s", nombreTarea1);
      getchar();
      printf("(Tarea Procedente): ");
      scanf("%30[^\n]s", nombreTarea2);
    
  }
  
  Pair *tareaPrecedente = searchMap(mapa,nombreTarea1);
  Pair *tareaPorHacer = searchMap(mapa,nombreTarea2);
  Tarea *tareaAux = tareaPrecedente->value;
  Tarea *tareaAux2 = tareaPorHacer->value;

  tareaAux2 -> precede = true;
  pushFront(tareaAux2->precedencia,tareaAux);

  stack_push(stack -> historial, "2");
  
  printf("\nPrecedencia establecida!!\n\n");
  
  return;
}

// 3. 
void mostrarTareasPendientes(HashMap *mapa)
{  
  Pair* aux = firstMap(mapa);
  
  Heap *pendientes =createHeap();
  HashMap * mapaAux = copiarMapa(mapa);
  

  while (true)
  {
    poblarHeap(mapaAux, pendientes);
    
    if(heap_size(pendientes) == 0) 
    {
      puts("");
      return;
    }
    
    Pair *raizHeap = searchMap(mapaAux, heap_top(pendientes));
    
    Tarea *tareaAux = raizHeap -> value;

    Pair *parAux = searchMap(mapa, heap_top(pendientes));
    Tarea *tareasOriginales = parAux -> value;

    char *nombreAux = tareaAux ->nombre;

    printf("\n%s (Prioridad: %d)", tareasOriginales -> nombre, tareasOriginales -> prioridad); 
    if(tareasOriginales -> precede == true)
    {
      char *nombreLista = firstList(tareasOriginales -> precedencia);
      
      printf(" - Precedente:"); 
      while(nombreLista != NULL)
      {
        printf(" %s", nombreLista);
        nombreLista = nextList(tareasOriginales -> precedencia);
      }
      puts("");
    }

    eraseMap(mapaAux, tareaAux -> nombre);
    heap_pop(pendientes);

    Pair *aux = firstMap(mapaAux);
    
    while(aux != NULL)
    {
      Tarea *tareaList = aux -> value;
      char *nombreLista = firstList(tareaList -> precedencia);

      while(nombreLista != NULL)
      {
        if(strcmp(nombreAux, nombreLista) == 0) 
        {
          popCurrent(tareaList -> precedencia);
        }
        nombreLista = nextList(tareaList -> precedencia);
      }
      
      if(firstList(tareaList -> precedencia) == NULL) tareaList -> precede = false;
      aux = nextMap(mapaAux);
    }
  }
  reiniciarheap(mapa,pendientes);
}


// 4.
void marcarTarea(HashMap *mapa, Pila *stack)
{
  Tarea *tareaAux;



  
  stack_push(stack -> historial, "4");
}

// 5.
void deshacerAccion(HashMap *mapa, Pila *stack, int *registrada)
{ 
  char *datoRegistro = stack_pop(stack -> historial);
  printf("\n%c\n\n", *datoRegistro);

  
  if(strcmp(datoRegistro, "0") == 0)
  {
    printf("\nNo se pueden deshacer más acciones. \n\n");
    stack_push(stack -> historial, "0");
  }
  else 
  if(strcmp(datoRegistro, "1") == 0)
  {
    Pair *tareaAux = stack_pop(stack -> agregadas);

     printf("\nLa última acción fue deshecha y la tarea ' %s ' fue eliminada!\n\n", tareaAux -> key);
    
    eraseMap(mapa, tareaAux -> key);
    (*registrada)--;
  }
  /*
  
  else 
  if(strcmp(datoRegistro, "2") == 0)
  {
    char *item = stack_pop(jugadorAux -> registroItem);
    insertMap(jugadorAux -> items, item, NULL);
    
    printf("\nLa última acción fue deshecha y el ítem ' %s ' fue agregado!\n\n", item);
    jugadorAux->numeroItems++;
  }
  else 
  if(strcmp(datoRegistro, "4") == 0)
  {
    //usar resgistroSum para restarle lo previamente sumado
    char *auxSum = stack_pop(jugadorAux -> registroSum);
    int numSum = atoi(auxSum);
    jugadorAux -> puntosHabilidad = jugadorAux -> puntosHabilidad - numSum;
    printf("\nLa última acción fue deshecha y al jugador se le disminuyeron ' %i ' puntos!\n\n", numSum);
  }
  */
  
}

// 6.
void importarTareas(HashMap *mapa)
{
  printf("Funcion 6\n\n");
}

// Programa principal
int main() 
{
  /*
                      ____ _             _           _             
                     / ___(_) __ _  __ _| |__  _   _| |_ ___     
                    | |  _| |/ _` |/ _` | '_ \| | | | __/ _ \     
                    | |_| | | (_| | (_| | |_) | |_| | ||  __/     
                     \____|_|\__, |\__,_|_.__/ \__, |\__\___| 
                             |___/             |___/                                                        
  */

  HashMap *mapa = createMap(10001);

  

  Pila *stack = malloc(sizeof(Pila));
  stack -> historial = stack_create();
  stack_push(stack -> historial, "0");
  stack -> agregadas = stack_create();
  stack -> marcadas = stack_create();
  
  
  int user_continue = 1;

  int registrada = 0;

  printf("Bienvenido! :D\n");

  while(user_continue)
  {
    menu();
    
    int opcion = 0;
    
    validarOpcion(&opcion); // Validamos que opción sea un número.
    
    if(registrada == 0 && opcion != 1 && opcion != 6 && opcion != 7 && opcion != 2)
    {
      printf("No hay tareas agregadas, debe registrar una primero.\n");
    }
    else
    switch(opcion)
    {
      case 1 :
        agregarTarea(mapa, stack, &registrada);
        validar(&user_continue);
        break;

      case 2 :
        if(registrada < 2) 
        {
          printf("Debe tener al menos dos tareas agregadas para establecer una precedencia.\n");
          break;
        }
        else
        {
          establecerPrecedencia(mapa,stack);
          validar(&user_continue);
          break;
        }
          
      case 3 :
        mostrarTareasPendientes(mapa);
        validar(&user_continue);
        break;

      case 4 :
        marcarTarea(mapa, stack);
        validar(&user_continue);
        break;

      case 5 :
        deshacerAccion(mapa, stack, &registrada);
        validar(&user_continue);
        break;

      case 6 :
        importarTareas(mapa);
        validar(&user_continue);
        break;
      
      case 7 :
        printf("Gracias por usar el programa, adiós!");
        return 0;
    }
    
  }
}