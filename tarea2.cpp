#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <ctype.h>
using namespace std;

typedef struct s_contenedor{
    string ip;
    string trans;
    string rec;
    string perd;
    string estado;
}CONTENEDOR;
void manejoDatos(string str, string ip);
void ping(string ip, size_t pos, string pk);
vector<CONTENEDOR> contenedor;

int main(int argc, char *argv[] ) {
    //verifico que se ingresen la cantidad de parametros correctos
    if(argc != 3){
        cout << "ingrese el <nombre del archivo>  <catidad de paquetes>" << endl;
        return 0;
    }
    //verifico que la cantidad de paquetes sea un numero
    if(isdigit(*argv[2]) == 0){
        cout << "la cantidad de paquetes debe ser un numero" << endl;
        return 0;
    }
    //verifico que el archivo exista
    ifstream archivo (argv[1]); //abro el archivo
    if(!archivo.is_open()){
    cout<<"no se puede abrir el archivo o direccion erronea";
	return 0;	
	}
    int cantidad=0; //contador para la cantidad de lineas 
    vector<string>ip; //vector para guardar las ips
    string line; //variable para guardar las lineas del archivo (ips por linea)
    int nPaquetes=  atoi(argv[2]);
    //leo el archivo y guardo las ips en el vector y cuento la cantidad de ips
    while(getline(archivo,line)){
     ip.push_back(line);
     cantidad++;
	}
    archivo.close();
    ostringstream oss;
    oss<<nPaquetes;
    //creo los hilos
	thread threads[cantidad];
    // creo un hilo por cada ip
	for (size_t i = 0 ; i < ip.size(); ++i){
	threads[i] = thread(ping,ip[i],i,oss.str());
	}
    //espero a que terminen los hilos
	for (int i=0; i< cantidad; i++) {
    threads[i].join();
  	}	
    //imprimo los resultados
    cout<<"IP"<<"\t\t\t"<<"Trans."<<"\t"<<"Rec."<<"\t"<<"Per."<<"\t"<<"Estado"<<endl;
    cout<<"------------------------------------------------------------"<<endl;
    for (size_t i = 0; i < contenedor.size();i++ )
    {
        cout<<contenedor[i].ip << "\t\t";
        cout<<contenedor[i].rec << "\t";
        cout<<contenedor[i].perd << "\t";
        cout<<contenedor[i].estado << "\n";
    }
  return 0;
}
//funcion ping
void ping(string ip, size_t pos, string pk){
    int count=0; //contador para saber la 4ta linea del buffer
	string ping=(string) "ping -q -c"+ pk.c_str()+" "+ ip; //comando ping
    //creo el buffer
    FILE *fp;
    char buffer[1024];
    fp = popen(ping.c_str(), "r"); // popen ejecuta el comando ping
    CONTENEDOR packaje;
    //leo el buffer (salida del comando ping)
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        count++;
        //veo la 4ta linea del buffer (la que tiene la informacion de los paquetes)
        if(count==4){  
            //envio la linea a la funcion manejoDatos para que la procese en conjunto con la ip
            manejoDatos(buffer,ip);
        }
    }
    pclose(fp);
    
}
void manejoDatos(string str, string ip){
    CONTENEDOR packaje;
    string transmited = "transmitted"; //variable para buscar la palabra transmitted
    string p_tras = str.substr(0,(str.find(transmited)-8)); //guardo la cantidad de paquetes transmitidos
    string recive= "received";//variable para buscar la palabra received
    string p_rec = str.substr(str.find(transmited)+13,str.find(recive)-(str.find(transmited)+13));//guardo la cantidad de paquetes recibidos
    //guardo la informacion en el contenedor
    packaje.ip=ip;
    packaje.trans=p_tras;
    packaje.rec=p_rec;
    packaje.perd=to_string(stoi(p_tras)-stoi(p_rec));//guardo la cantidad de paquetes perdidos (transmitidos - recibidos)
    //verifico si la ip esta activa o no
    if(stoi(p_rec)==0){
    packaje.estado="DOWN";
    }else{  
    packaje.estado="UP";
    }
    //guardo el contenedor en el vector
    contenedor.push_back(packaje);
}

