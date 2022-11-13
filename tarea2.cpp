//Matias Roman
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

class Contenedor{
    public:
    Contenedor();
    Contenedor(string ip, string trans, string rec, string perd, string estado);

    void setsIp(string ip);
    void setsTrans(string trans );
    void setsRec( string rec);
    void setsPerd( string perd);
    void setsEstado( string estado);
    void imprimir();

    private:
    string ip;
    string trans;
    string rec;
    string perd;
    string estado;
};
Contenedor::Contenedor(){
    ip = "";
    trans="";
    rec="";
    perd="";
    estado="";

}

Contenedor::Contenedor(string ip, string trans, string rec, string perd, string estado){
    this->ip=ip;
    this->trans=trans;
    this->rec=rec;
    this->perd=perd;
    this->estado=estado;
}
void Contenedor:: setsIp(string ip){
    this->ip=ip;
}
void Contenedor:: setsTrans(string trans){
    this->trans=trans;
}
void Contenedor:: setsRec(string rec){
    this->rec=rec;
}
void Contenedor:: setsPerd(string perd){
    this->perd=perd;
}
void Contenedor:: setsEstado(string estado){
    this->estado=estado;
}
void Contenedor:: imprimir(){
    cout<<this->ip <<"\t\t";
    cout<<this->trans <<"\t";
    cout<<this->rec <<"\t";
    cout<<this->perd <<"\t";
    cout<<this->estado <<"\t" <<"\n";
}
//funcion para extraer los datos de ip , transmisiones, recibidas y perdidas, estado
void manejoDatos(string str, string ip,size_t pos);
//funcion que se encarga de hacer el ping 
void ping(string ip, size_t pos, string pk);
//vector que contiene los datos de las ips
vector<Contenedor> lista;
pthread_mutex_t mutex;

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
    pthread_mutex_init(&mutex, NULL);
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
    for (size_t i = 0; i < lista.size();i++ )
    {
        lista[i].imprimir();
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
    fp = popen(ping.c_str(), "r"); // popen ejecuta el comando ping (crea un ejecutable (nombre ejecutable, modo de ejecucion, comando)) 
    //leo el buffer (salida del comando ping)
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        count++;
        //veo la 4ta linea del buffer (la que tiene la informacion de los paquetes)
        if(count==4){  
            //envio la linea a la funcion manejoDatos para que la procese en conjunto con la ip
            manejoDatos(buffer,ip,pos);
        }
    }
    pclose(fp);
    
}
void manejoDatos(string str, string ip, size_t pos){
    Contenedor contenedor;
    string transmited = "transmitted"; //variable para buscar la palabra transmitted
    string p_tras = str.substr(0,(str.find(transmited)-8)); //guardo la cantidad de paquetes transmitidos
    string recive= "received";//variable para buscar la palabra received
    string p_rec = str.substr(str.find(transmited)+13,str.find(recive)-(str.find(transmited)+13));//guardo la cantidad de paquetes recibidos
    string perdidos= to_string(stoi(p_tras)-stoi(p_rec)); //guardo la cantidad de paquetes perdidos (transmitidos - recibidos)
    //guardo la informacion en el contenedor
    contenedor.setsIp(ip);
    contenedor.setsTrans(p_tras);
    contenedor.setsRec(p_rec);
    contenedor.setsPerd(perdidos);
    //verifico si la ip esta activa o no vasta un solo paquete recibido para que este activa
    if(stoi(p_rec)==0){
        contenedor.setsEstado("DOWN");
    }else{
        contenedor.setsEstado("UP");
    }
    //ocupo el mutex para que las hebras no pusheen al mismo tiempo
    pthread_mutex_lock(&mutex);
     //guardo el contenedor en el vector
    lista.push_back(contenedor);

    pthread_mutex_unlock(&mutex);
}

