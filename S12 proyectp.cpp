#include <iostream>
#include <string>
#include <fstream>
using namespace std;

// Variables globales para la gestion de memoria
const int memoriaTotal = 1024;
int memoriaUsada = 0;

// Estructura que define los datos de un proceso
struct Proceso{
    int id;
    string nombre;
    int prioridad;
    string estado;
    int memoria;
};

// Nodo para la lista enlazada de procesos
struct NodoProceso{
    Proceso dato;
    NodoProceso* siguiente;
};

// Nodo para la cola de la CPU (apunta a un proceso de la lista)
struct NodoCola{
    Proceso* proceso;
    NodoCola* siguiente;
};

// Nodo para la pila de memoria
struct NodoPila{
    int idProceso;
    int tamanio;
    NodoPila* siguiente;
};


// FUNCIONES AUXILIARES


// Pausa la consola hasta que el usuario presione Enter
void pausa(){
    cout<<endl<<"Presione Enter para continuar...";
    cin.ignore(); // Limpia el buffer del 'Enter' anterior
    cin.get();    // Espera un nuevo 'Enter'
}

// Dibuja una linea de separacion en la consola
void linea(int n){
    for(int i=0;i<n;i++){
    	cout<<"=";
	}
    cout<<endl;
}

// Imprime en pantalla los datos de un proceso especifico
void mostrarProceso(Proceso p){
    cout<<"ID: "<<p.id<<" | Nombre: "<<p.nombre<<" | Prioridad: "<<p.prioridad<<" | Estado: "<< p.estado<<" | Memoria: "<<p.memoria<<endl;
}


// GESTOR DE PROCESOS (LISTA ENLAZADA)


// Busca un proceso por su ID en la lista enlazada. Retorna el nodo o NULL si no existe.
NodoProceso* buscarPorId(NodoProceso* cabeza, int id){
    NodoProceso* aux = cabeza;
    while (aux != NULL) {
        if (aux->dato.id == id) return aux;
        aux = aux->siguiente;
    }
    return NULL;
}

// Busca un proceso por su nombre en la lista enlazada.
NodoProceso* buscarPorNombre(NodoProceso* cabeza, string nombre){
    NodoProceso* aux = cabeza;
    while (aux != NULL) {
        if (aux->dato.nombre == nombre) return aux;
        aux = aux->siguiente;
    }
    return NULL;
}

// Inserta un nuevo proceso al final de la lista enlazada.
bool insertarProceso(NodoProceso*& cabeza, Proceso p){
    // Verifica que el ID no este duplicado
    if (buscarPorId(cabeza, p.id)!=NULL){
    	return false;
	}
    
    // Crea el nuevo nodo en memoria dinamica
    NodoProceso* nuevo=new NodoProceso;
    nuevo->dato=p;
    nuevo->siguiente=NULL;
    
    // Si la lista esta vacia, el nuevo nodo es la cabeza
    if (cabeza==NULL){
        cabeza=nuevo;
    } 
	else{
        // Si no, recorre hasta el final y enlaza el nuevo nodo
        NodoProceso* aux=cabeza;
        while (aux->siguiente!=NULL){
        	aux=aux->siguiente;
		}
        aux->siguiente=nuevo;
    }
    return true;
}

// Elimina un proceso de la lista enlazada segun su ID.
bool eliminarProceso(NodoProceso*& cabeza, int id){
    NodoProceso* actual = cabeza;
    NodoProceso* anterior = NULL;
    
    // Busca el nodo a eliminar
    while (actual != NULL && actual->dato.id != id){
        anterior = actual;
        actual = actual->siguiente;
    }
    
    // Si no lo encuentra, retorna falso
    if (actual == NULL){
    	return false;
	}
    
    // Si es el primer nodo, la cabeza pasa al siguiente
    if (anterior == NULL){
    	cabeza=actual->siguiente;
	}
    // Si esta en medio, el anterior salta al siguiente del actual
    else anterior->siguiente=actual->siguiente;
    
    delete actual; // Libera la memoria del nodo eliminado
    return true;
}

// Actualiza la prioridad de un proceso
void modificarPrioridad(NodoProceso* nodo, int nuevaPrioridad){
    nodo->dato.prioridad=nuevaPrioridad;
}

// Cambia el estado de un proceso seleccionandolo de un menu
void cambiarEstadoProceso(NodoProceso* cabeza){
    int id;
    cout<<"ID del proceso: "; cin>>id;
    NodoProceso* aux=buscarPorId(cabeza, id);

    if (aux==NULL) {
        cout<<"Proceso no encontrado."<<endl;
    } else {
        string estados[4]={"Nuevo", "Listo", "Ejecutando", "Terminado"};
        for (int i = 0; i < 4; i++){
        	cout<<(i + 1)<<". "<<estados[i]<<endl;
		}

        int opcion;
        
        // Validacion de opcion
        do {
            cout<<"Opcion (1-4): "; cin>>opcion;
            if(opcion<1 || opcion>4){
            	cout<<"__Error"<<endl;
			}
        } while(opcion<1 || opcion>4);
        
        // Asigna el estado seleccionado
        aux->dato.estado=estados[opcion - 1];
        cout<<"Estado actualizado: "<<aux->dato.estado<<endl;
    }
}

// Recorre e imprime todos los procesos registrados en la lista
void listarProcesos(NodoProceso* cabeza){
    if (cabeza==NULL) {
        cout<<"No hay procesos registrados."<<endl;
        return;
    }
    NodoProceso* aux=cabeza;
    while(aux!=NULL){
		mostrarProceso(aux->dato);
	    aux=aux->siguiente;
    }
}


// PLANIFICADOR DE CPU (COLA DE PRIORIDAD)


// Verifica si un proceso ya esta en la cola para evitar duplicados
bool enCola(NodoCola* frente, int id){
    NodoCola* aux=frente;
    while (aux!=NULL){
        if (aux->proceso->id==id){
        	return true;
		}
        aux=aux->siguiente;
    }
    return false;
}

// Inserta un proceso en la cola segun su prioridad
bool encolar(NodoCola*& frente, NodoCola*& final, Proceso* p){
    if (enCola(frente, p->id)){
    	return false;
	} 
    
    // Crea el nodo en la cola (solo guarda el puntero al proceso)
    NodoCola* nuevo=new NodoCola;
    nuevo->proceso=p;
    nuevo->siguiente=NULL;
    p->estado="Listo";

    // Si la cola esta vacia
    if (frente==NULL) {
        frente=nuevo;
        final=nuevo;
    } 
    // Si tiene mayor prioridad que el primero, pasa a ser el nuevo frente
	else if(p->prioridad > frente->proceso->prioridad){
        nuevo->siguiente = frente;
        frente = nuevo;
    } 
    // Si no, busca su lugar segun la prioridad
	else{
        NodoCola* aux=frente;
        while (aux->siguiente!=NULL&&aux->siguiente->proceso->prioridad>=p->prioridad){
            aux = aux->siguiente;
        }
        nuevo->siguiente=aux->siguiente;
        aux->siguiente=nuevo;
        if (nuevo->siguiente==NULL){
        	final = nuevo; // Si quedo al final, actualiza el puntero 'final'
		}
    }
    return true;
}

// Saca el proceso del frente de la cola y simula su ejecucion
void desencolar(NodoCola*& frente, NodoCola*& final) {
    if (frente==NULL) {
        cout<<"La cola esta vacia. No hay procesos para ejecutar."<<endl;
        return;
    }
    NodoCola* aux=frente;
    frente=frente->siguiente;
    if (frente==NULL){
    	final=NULL; // Si la cola quedo vacia, 'final' tambien es NULL
	}
    
    // Simula el cambio de estado del proceso
    aux->proceso->estado="Ejecutando";
    cout<<"Ejecutando proceso:"<<endl;
    mostrarProceso(*(aux->proceso));
    aux->proceso->estado="Terminado";
    cout<<"El proceso ha terminado."<<endl;
    
    delete aux; // Libera el nodo de la cola (no el proceso en si)
}

// Muestra los procesos en la cola respetando el orden de prioridad
void mostrarCola(NodoCola* frente) {
    if (frente==NULL) {
        cout<<"La cola esta vacia."<<endl;
        return;
    }
    int posicion = 1;
    NodoCola* aux = frente;
    while (aux != NULL) {
        cout << posicion << ". ID: " << aux->proceso->id
             << " | " << aux->proceso->nombre
             << " | Prioridad: " << aux->proceso->prioridad << endl;
        posicion++;
        aux = aux->siguiente;
    }
}

// Busca un proceso en la cola usando una cola auxiliar para no perder el orden
int buscarEnCola(NodoCola*& frente, NodoCola*& final, int id) {
    NodoCola* auxFrente=NULL;
    NodoCola* auxFinal=NULL;
    int posicion=0;
    int encontrada=0;
    
    // Desarma la cola original y la pasa a la auxiliar
    while(frente!=NULL){
        NodoCola* nodo=frente;
        frente=frente->siguiente;
        nodo->siguiente=NULL;
        posicion++;
        if(nodo->proceso->id==id){
        	encontrada=posicion;
		} 
        
        if(auxFrente==NULL){
            auxFrente=nodo;
            auxFinal=nodo;
        } 
		else{
            auxFinal->siguiente=nodo;
            auxFinal=nodo;
        }
    }
    // Restaura la cola original
    frente=auxFrente;
    final=auxFinal;
    return encontrada;
}

// Elimina un proceso especifico de la cola (usado al cambiar prioridad)
bool quitarDeCola(NodoCola*& frente, NodoCola*& final, int id){
    NodoCola* actual=frente;
    NodoCola* anterior=NULL;
    while (actual!=NULL&&actual->proceso->id!=id){
        anterior=actual;
        actual=actual->siguiente;
    }
    if (actual==NULL){
    	return false;
	}
    
    if (anterior==NULL){
     	frente=actual->siguiente;	
	}
    else{
    	anterior->siguiente=actual->siguiente;
	}
    
    if (actual==final){
    	final=anterior;
	}
    delete actual;
    return true;
}

// Muestra los procesos terminados (atendidos) y los que estan en la cola (pendientes)
void listarAtendidosYPendientes(NodoProceso* cabeza, NodoCola* frente){
    int atendidos=0;
    int pendientes=0;

    cout << ""<<endl<<"--- PROCESOS ATENDIDOS ---"<<endl;
    NodoProceso* aux=cabeza;
    while (aux!=NULL){
        if (aux->dato.estado=="Terminado"){
            cout <<"ID: "<<aux->dato.id <<" | "<<aux->dato.nombre<<" | Prioridad: "<< aux->dato.prioridad<<endl;
            atendidos++;
        }
        aux = aux->siguiente;
    }
    if (atendidos == 0){
    	cout<<"Ningun proceso ha sido atendido."<<endl;
	}

    cout <<endl<<"--- PROCESOS PENDIENTES ---"<<endl;
    NodoCola* actual = frente;
    while (actual != NULL){
        pendientes++;
        cout<<pendientes<<". ID: "<<actual->proceso->id<<" | "<<actual->proceso->nombre<<" | Prioridad: "<<actual->proceso->prioridad<<endl;
        actual = actual->siguiente;
    }
    if (pendientes == 0){
    	cout<<"No hay procesos pendientes."<<endl;
	}

    cout<<endl<<"Atendidos: "<<atendidos<<"  Pendientes: "<<pendientes<<endl;
}


// GESTOR DE MEMORIA (PILA)

// Verifica si un proceso tiene bloques de memoria asignados
bool tieneBloques(NodoPila* cima, int id){
    NodoPila* aux=cima;
    while(aux!=NULL){
        if(aux->idProceso==id){
        	return true;
		}
        aux=aux->siguiente;
    }
    return false;
}

// Apila (push) un nuevo bloque de memoria para un proceso
bool asignarMemoria(NodoPila*& cima, int id, int tamanio){
    // Verifica que no exceda la memoria total
    if(memoriaUsada+tamanio>memoriaTotal){
    	return false;
	}
    
    NodoPila* nuevo=new NodoPila;
    nuevo->idProceso=id;
    nuevo->tamanio=tamanio;
    nuevo->siguiente=cima;
    cima=nuevo;
    memoriaUsada=memoriaUsada+tamanio;
    return true;
}

// Desapila (pop) el ultimo bloque de memoria asignado
void liberarMemoria(NodoPila*& cima){
    if (cima==NULL){
        cout<<"La pila esta vacia. No hay bloques para liberar."<<endl;
        return;
    }
    NodoPila* aux=cima;
    cima=cima->siguiente;
    memoriaUsada=memoriaUsada - aux->tamanio;
    cout<<"Bloque liberado -> Proceso ID: "<<aux->idProceso<<" | Tamano: "<<aux->tamanio<<endl;
    delete aux;
    if(cima==NULL){
    	cout<<"La pila quedo vacia."<<endl;
	}
}

// Muestra el estado actual de la memoria (total, usada y disponible)
void estadoMemoria(NodoPila* cima){
    cout<<"Memoria total:\t\t"<<memoriaTotal<<endl;
    cout<<"Memoria usada:\t\t"<< memoriaUsada<<endl;
    cout<<"Memoria disponible:\t"<< memoriaTotal-memoriaUsada<<endl;
    if(cima==NULL){
        cout<<"No hay bloques asignados."<<endl;
    }
	else{
        cout<<"Bloques (de cima a base):"<<endl;
        NodoPila* aux=cima;
        while(aux!=NULL){
            cout<<"  Proceso ID: "<<aux->idProceso<<" | Tamano: "<<aux->tamanio<<endl;
            aux=aux->siguiente;
        }
    }
}

// Busca cuantos bloques y cuanta memoria tiene un proceso en la pila
void buscarBloques(NodoPila*& cima, int id){
    NodoPila* auxCima=NULL;
    int cantidad=0;
    int total=0;
    // Desapila a una pila auxiliar para revisar
    while(cima!=NULL){
        NodoPila* nodo=cima;
        cima=cima->siguiente;
        if(nodo->idProceso==id){
            cantidad++;
            total=total+nodo->tamanio;
        }
        nodo->siguiente=auxCima;
        auxCima=nodo;
    }
    // Devuelve los nodos a la pila original
    while(auxCima!=NULL){
        NodoPila* nodo=auxCima;
        auxCima=auxCima->siguiente;
        nodo->siguiente=cima;
        cima=nodo;
    }
    if(cantidad==0){
        cout<<"El proceso "<<id<<" no tiene bloques asignados."<<endl;
    }
	else{
        cout<<"El proceso "<<id<<" tiene "<<cantidad<<" bloque(s), con un total de "<<total<<" de memoria."<<endl;
    }
}

// Consulta la memoria total que consume un proceso (buscando en lista y pila)
void consultarMemoriaProceso(NodoProceso* cabeza, NodoPila*& cima, int id){
    NodoProceso* p=buscarPorId(cabeza, id);
    if(p==NULL){
        cout<<"No se encontro el proceso."<<endl;
        return;
    }
    NodoPila* auxCima=NULL;
    int total=0;
    int bloques=0;
    while(cima!=NULL){
        NodoPila* nodo=cima;
        cima=cima->siguiente;
        if(nodo->idProceso==id){
            total=total+nodo->tamanio;
            bloques++;
        }
        nodo->siguiente=auxCima;
        auxCima=nodo;
    }
    while (auxCima!=NULL){
        NodoPila* nodo=auxCima;
        auxCima=auxCima->siguiente;
        nodo->siguiente=cima;
        cima=nodo;
    }
    cout<<"Proceso: "<<p->dato.nombre<<"  ID: "<<id<<endl;
    if(bloques==0){
        cout<<"No tiene memoria asignada."<<endl;
    }
	else{
        cout<<"Memoria consumida: "<<total<<" ("<<bloques<<" bloque(s))"<<endl;
    }
}


// PERSISTENCIA (ARCHIVOS - API STREAM)

// Guarda la lista de procesos en un archivo de texto
void guardarProcesos(NodoProceso* cabeza){
    ofstream archivo("procesos.txt"); 
    
    if (!archivo.is_open()){
    	return;
	}

    NodoProceso* aux=cabeza;
    
    while(aux!=NULL){
        archivo<<aux->dato.id<<" "<<aux->dato.prioridad<<" "<<aux->dato.memoria<<" "<<aux->dato.estado<<endl;
        archivo<<aux->dato.nombre<<endl;
        aux=aux->siguiente;
    }
    archivo.close();
}

// Guarda los IDs de los procesos en la cola
void guardarCola(NodoCola* frente){
    ofstream archivo("cola.txt");
    if(!archivo.is_open()){
    	return;
	} 
    NodoCola* aux =frente;
    while(aux!=NULL){
        archivo<<aux->proceso->id<<endl;
        aux=aux->siguiente;
    }
    archivo.close();
}

// Guarda los bloques de memoria en la pila
void guardarPila(NodoPila* cima) {
    ofstream archivo("pila.txt");
    if (!archivo.is_open()) return;
    NodoPila* aux = cima;
    while (aux != NULL) {
        archivo << aux->idProceso << " " << aux->tamanio << endl;
        aux = aux->siguiente;
    }
    archivo.close();
}

// Carga los procesos desde el archivo a la lista enlazada
void cargarProcesos(NodoProceso*& cabeza) {
    ifstream archivo("procesos.txt"); 
    if (!archivo.is_open()) return;
    Proceso p;
    while (archivo >> p.id >> p.prioridad >> p.memoria >> p.estado) {
        archivo.ignore(); 
        getline(archivo, p.nombre); 
        insertarProceso(cabeza, p);
    }
    archivo.close();
}

// Carga los IDs de la cola y vuelve a enlazar los procesos
void cargarCola(NodoProceso* cabeza, NodoCola*& frente, NodoCola*& final) {
    ifstream archivo("cola.txt");
    if (!archivo.is_open()) return;
    int id;
    while (archivo >> id) {
        NodoProceso* nodo = buscarPorId(cabeza, id);
        if (nodo != NULL) encolar(frente, final, &(nodo->dato));
    }
    archivo.close();
}

// Carga los bloques de memoria del archivo a la pila
void cargarPila(NodoPila*& cima) {
    ifstream archivo("pila.txt");
    if (!archivo.is_open()) return;
    NodoPila* aux = NULL;
    int id, tam;
    while (archivo >> id >> tam) {
        NodoPila* nuevo = new NodoPila;
        nuevo->idProceso = id;
        nuevo->tamanio = tam;
        nuevo->siguiente = aux;
        aux = nuevo;
        memoriaUsada = memoriaUsada + tam;
    }
    archivo.close();
    // Invierte la pila para restaurar el orden original
    while (aux != NULL) {
        NodoPila* nodo = aux;
        aux = aux->siguiente;
        nodo->siguiente = cima;
        cima = nodo;
    }
}

// Llama a las tres funciones de guardado
void guardarTodo(NodoProceso* cabeza, NodoCola* frente, NodoPila* cima) {
    guardarProcesos(cabeza);
    guardarCola(frente);
    guardarPila(cima);
}

// Llama a las tres funciones de carga
void cargarTodo(NodoProceso*& cabeza, NodoCola*& frente, NodoCola*& final, NodoPila*& cima) {
    cargarProcesos(cabeza);
    cargarCola(cabeza, frente, final);
    cargarPila(cima);
}

// Libera toda la memoria dinamica antes de cerrar el programa (Evita fugas)
void liberarTodo(NodoProceso*& cabeza, NodoCola*& frente, NodoCola*& final, NodoPila*& cima) {
    while (frente != NULL) {
        NodoCola* aux = frente;
        frente = frente->siguiente;
        delete aux;
    }
    final = NULL;
    while (cima != NULL) {
        NodoPila* aux = cima;
        cima = cima->siguiente;
        delete aux;
    }
    memoriaUsada = 0;
    while (cabeza != NULL) {
        NodoProceso* aux = cabeza;
        cabeza = cabeza->siguiente;
        delete aux;
    }
}


// MENUS


// Menu interactivo para la gestion de procesos (Lista)
void menuProcesos(NodoProceso*& cabeza, NodoCola*& frente, NodoCola*& final, NodoPila* cima) {
    int opp;
    do {
        cout << ""<<endl<<"--- GESTOR DE PROCESOS ---"<<endl;
        cout << "1. Insertar proceso"<<endl;
        cout << "2. Eliminar proceso"<<endl;
        cout << "3. Buscar por ID"<<endl;
        cout << "4. Buscar por nombre"<<endl;
        cout << "5. Modificar prioridad"<<endl;
        cout << "6. Cambiar estado"<<endl;
        cout << "7. Listar procesos"<<endl;
        cout << "8. Volver"<<endl;
        
        do {
            cout << "Ingrese una opcion: "; cin >> opp;
            if(opp < 1 || opp > 8) cout << "__Error"<<endl;
        } while(opp < 1 || opp > 8);

        switch (opp) {
            case 1: {
                Proceso p;
                cout << "ID: "; cin >> p.id;
                cout << "Nombre: "; cin >> p.nombre;
                cout << "Prioridad (1-10): "; cin >> p.prioridad;
                cout << "Memoria requerida: "; cin >> p.memoria;
                p.estado = "Nuevo";
                
                if (insertarProceso(cabeza, p)) cout << "Proceso registrado correctamente."<<endl;
                else cout << "Error: ya existe un proceso con ese ID."<<endl;
                pausa();
                break;
            }
            case 2: {
                int id;
                cout << "ID a eliminar: "; cin >> id;
                if (enCola(frente, id) || tieneBloques(cima, id)) {
                    cout << "No se puede eliminar: esta en la cola o tiene memoria."<<endl;
                } else if (eliminarProceso(cabeza, id)) {
                    cout << "Proceso eliminado."<<endl;
                } else {
                    cout << "No se encontro el proceso."<<endl;
                }
                pausa();
                break;
            }
            case 3: {
                int id;
                cout << "ID a buscar: "; cin >> id;
                NodoProceso* nodo = buscarPorId(cabeza, id);
                if (nodo != NULL) mostrarProceso(nodo->dato);
                else cout << "No se encontro el proceso."<<endl;
                pausa();
                break;
            }
            case 4: {
                string nombre;
                cout << "Nombre a buscar: "; cin >> nombre;
                NodoProceso* nodo = buscarPorNombre(cabeza, nombre);
                if (nodo != NULL) mostrarProceso(nodo->dato);
                else cout << "No se encontro el proceso."<<endl;
                pausa();
                break;
            }
            case 5: {
                int id;
                cout << "ID del proceso: "; cin >> id;
                NodoProceso* nodo = buscarPorId(cabeza, id);
                if (nodo == NULL) {
                    cout << "No se encontro el proceso."<<endl;
                } else {
                    int nueva;
                    cout << "Nueva prioridad (1-10): "; cin >> nueva;
                    modificarPrioridad(nodo, nueva);
                    // Si el proceso esta en la cola, se reordena automaticamente
                    if (enCola(frente, id)) {
                        quitarDeCola(frente, final, id);
                        encolar(frente, final, &(nodo->dato));
                        cout << "Prioridad actualizada y cola reordenada."<<endl;
                    } else {
                        cout << "Prioridad actualizada."<<endl;
                    }
                }
                pausa();
                break;
            }
            case 6:
                cambiarEstadoProceso(cabeza);
                pausa();
                break;
            case 7:
                listarProcesos(cabeza);
                pausa();
                break;
        }
    } while (opp!=8);
}

// Menu interactivo para el planificador de la CPU (Cola)
void menuCpu(NodoProceso* cabeza, NodoCola*& frente, NodoCola*& final) {
    int opCpu;
    do {
        cout << ""<<endl<<"--- PLANIFICADOR DE CPU ---"<<endl;
        cout << "1. Encolar proceso"<<endl;
        cout << "2. Ejecutar siguiente proceso"<<endl;
        cout << "3. Mostrar cola"<<endl;
        cout << "4. Buscar proceso en la cola"<<endl;
        cout << "5. Listar atendidos y pendientes"<<endl;
        cout << "6. Volver"<<endl;
        
        do {
            cout << "Ingrese una opcion: "; cin >> opCpu;
            if(opCpu < 1 || opCpu > 6) cout << "__Error"<<endl;
        } while(opCpu < 1 || opCpu > 6);
        
        switch (opCpu) {
            case 1: {
                int id;
                cout << "ID del proceso a encolar: "; cin >> id;
                NodoProceso* nodo = buscarPorId(cabeza, id);
                if (nodo == NULL) cout << "No se encontro el proceso."<<endl;
                else if (nodo->dato.estado == "Terminado") cout << "Ese proceso ya termino."<<endl;
                else if (encolar(frente, final, &(nodo->dato))) cout << "Proceso encolado."<<endl;
                else cout << "El proceso ya esta en la cola."<<endl;
                pausa();
                break;
            }
            case 2:
                desencolar(frente, final);
                pausa();
                break;
            case 3:
                mostrarCola(frente);
                pausa();
                break;
            case 4: {
                int id;
                cout << "ID a buscar en la cola: "; cin >> id;
                int pos = buscarEnCola(frente, final, id);
                if (pos == 0) cout << "El proceso no esta en la cola."<<endl;
                else cout << "El proceso esta en la posicion " << pos << "."<<endl;
                pausa();
                break;
            }
            case 5:
                listarAtendidosYPendientes(cabeza, frente);
                pausa();
                break;
        }
    } while (opCpu != 6);
}

// Menu interactivo para el gestor de memoria (Pila)
void menuMemoria(NodoProceso* cabeza, NodoPila*& cima) {
    int opMemoria;
    do {
        cout << ""<<endl<<"--- GESTOR DE MEMORIA ---"<<endl;
        cout << "1. Asignar bloque (push)"<<endl;
        cout << "2. Liberar ultimo bloque (pop)"<<endl;
        cout << "3. Estado de la memoria"<<endl;
        cout << "4. Buscar bloques de un proceso"<<endl;
        cout << "5. Consultar memoria que consume un proceso"<<endl;
        cout << "6. Volver"<<endl;
        
        do {
            cout << "Ingrese una opcion: "; cin >> opMemoria;
            if(opMemoria < 1 || opMemoria > 6){
            	cout << "__Error"<<endl;
			}
        } while(opMemoria < 1 || opMemoria > 6);

        switch (opMemoria) {
            case 1: {
                int id;
                cout << "ID del proceso: "; cin >> id;
                NodoProceso* p = buscarPorId(cabeza, id);
                if (p == NULL) cout << "No se encontro el proceso."<<endl;
                else if (asignarMemoria(cima, id, p->dato.memoria)) {
                    cout << "Bloque de " << p->dato.memoria << " asignado a " << p->dato.nombre << "."<<endl;
                } else cout << "Memoria insuficiente."<<endl;
                pausa();
                break;
            }
            case 2:
                liberarMemoria(cima);
                pausa();
                break;
            case 3:
                estadoMemoria(cima);
                pausa();
                break;
            case 4: {
                int id;
                cout << "ID del proceso: "; cin >> id;
                buscarBloques(cima, id);
                pausa();
                break;
            }
            case 5: {
                int id;
                cout << "ID del proceso: "; cin >> id;
                consultarMemoriaProceso(cabeza, cima, id);
                pausa();
                break;
            }
            case 6:
            	break;
        }
    } while (opMemoria != 6);
}

// Menu interactivo para guardar y cargar los datos desde archivos
void menuArchivos(NodoProceso*& cabeza, NodoCola*& frente, NodoCola*& final, NodoPila*& cima) {
    int opArchivo;
    do {
        cout << ""<<endl<<"--- ARCHIVOS ---"<<endl;
        cout << "1. Guardar estado"<<endl;
        cout << "2. Cargar estado"<<endl;
        cout << "3. Volver"<<endl;
        
        do {
            cout << "Ingrese una opcion: "; cin >> opArchivo;
            if(opArchivo < 1 || opArchivo > 3){
            cout << "__Error"<<endl;	
			}
        } while(opArchivo < 1 || opArchivo > 3);

        switch (opArchivo) {
            case 1:
                guardarTodo(cabeza, frente, cima);
                cout << "Estado guardado."<<endl;
                pausa();
                break;
            case 2:
                // Limpia la memoria actual antes de cargar el archivo para evitar duplicados
                liberarTodo(cabeza, frente, final, cima);
                cargarTodo(cabeza, frente, final, cima);
                cout << "Estado cargado."<<endl;
                pausa();
                break;
        }
    } while (opArchivo != 3);
}

// Funcion principal que inicia el sistema
int main() {
    // Inicializa los punteros de control de las tres estructuras
    NodoProceso* cabeza = NULL;
    NodoCola* frente = NULL;
    NodoCola* final = NULL;
    NodoPila* cima = NULL;

    // Carga datos previos si es que existen en los archivos
    cargarTodo(cabeza, frente, final, cima);

    int op;
    do {
        cout << ""<<endl;
        linea(40);
        cout << "   SISTEMA DE GESTION DE PROCESOS"<<endl;
        linea(40);
        cout << "1. Gestor de procesos"<<endl;
        cout << "2. Planificador de CPU"<<endl;
        cout << "3. Gestor de memoria"<<endl;
        cout << "4. Archivos (guardar / cargar)"<<endl;
        cout << "5. Salir"<<endl;
        
        do {
            cout << "Ingrese una opcion: "; cin >> op;
            if(op<1 || op>5){
            	cout<<"__Error"<<endl;
			}
        } while(op<1 || op>5);

        switch (op) {
            case 1: 
				menuProcesos(cabeza, frente, final, cima); 
				break;
            case 2: 
				menuCpu(cabeza, frente, final); 
				break;
            case 3: 
				menuMemoria(cabeza, cima); 
				break;
            case 4: 
				menuArchivos(cabeza, frente, final, cima); 
				break;
            case 5: 
				cout << "Guardando estado y saliendo..."<<endl; 
				break;
        }
    } while (op!=5);

    // Al salir, guarda todo automaticamente y libera la memoria dinamica
    guardarTodo(cabeza, frente, cima);
    liberarTodo(cabeza, frente, final, cima);
    return 0;
}
