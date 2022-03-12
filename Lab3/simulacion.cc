/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Practica 03 - Simulación de Redes
Fecha: 24/10/17
Alumnos: Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: simulacion.cc
Descripcion: Este fichero contiene la funcion main y la funcion simulacion, encargadas de configurar el sistema, generar las graficas y ejecutar la simulacion
*/

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "Observador.h"
#include <ns3/gnuplot.h>

using namespace ns3;
#define NCSMA     9
#define N 		  21
#define RETARDO   "6560ns"
#define TASA      "100Mb/s"
#define TAMANO    500+N*20
#define INTERVALO "1s"
#define STOP      "100s"
#define RUN       "1s"
#define ERROR     0
#define MUESTRAS  30
#define TSTUDENT  2.0423
#define MAXREINTENTOS 20

NS_LOG_COMPONENT_DEFINE ("Simulacion");
void simulacion (uint32_t nCsma, Time delay, DataRate rate, uint32_t packetSize, Time tstop, Time tstart, std::string interval, double perror, uint32_t reintentos, Average<double> * acumIntentos, Average<double> * acumRetardos, Average<double> * acumPaquetes);

int
main (int argc, char *argv[])
{
	//Se establece la resolucion en nano segundos
	Time::SetResolution (Time::NS);

	GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));
	
	//Se establecen los valores por defecto para las variables
	uint32_t    nCsma = NCSMA;	 
	Time        delay (RETARDO);
	DataRate    rate (TASA);
	uint32_t    packetSize = TAMANO;
	Time        tstop (STOP);
	Time        tstart (RUN);
	std::string interval (INTERVALO);
	double      perror = ERROR;

	//Para configurar los valores de las variables mediante linea de comandos
	CommandLine cmd;
	cmd.AddValue ("nCsma", "número de nodos de la red local", nCsma);
	cmd.AddValue ("retardoProp", "retardo de propagación en el bus", delay);
	cmd.AddValue ("capacidad", "capacidad del bus", rate);
	cmd.AddValue ("perror", "probabilidad de error de bit, entre 0 y 1", perror);

	cmd.AddValue ("tamPaquete", "tamano de las SDU de aplicacion", packetSize);
	cmd.AddValue ("intervalo", "tiempo entre dos paquetes consecutivos enviados por el mismo cliente", interval);
	cmd.AddValue ("tstart", "instante de simulacion de inicio de las aplicaciones cliente", tstart);
	cmd.AddValue ("tstop", "intante de simulacion de fin de aplicaciones cliente y del servidor", tstop);

	cmd.Parse (argc, argv);

   // Debemos garantizar qu hay al menos un cliente y el servidor.
	if (nCsma <= 1)
		nCsma = 2;

	//Creamos las variables Gnuplot para generar las gráficas y configuramos el estilo
	Gnuplot2dDataset datos_medias1 ("medias");
	datos_medias1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	datos_medias1.SetErrorBars (Gnuplot2dDataset::Y);
	
	Gnuplot2dDataset datos_medias2 ("medias");
	datos_medias2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	datos_medias2.SetErrorBars (Gnuplot2dDataset::Y);
	
	Gnuplot2dDataset datos_medias3 ("medias");
	datos_medias3.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	datos_medias3.SetErrorBars (Gnuplot2dDataset::Y);
	
    // Se crea un objeto de tipo Average para cada variable a medir, los elementos del mismo serán de tipo double
    // Permitirá guardar los valores de interés obtenidos de la simulación
	
	Average<double> acumIntentos;
	Average<double> acumRetardos;
	Average<double> acumPaquetes;


	//Bucle para simular el sistema MUESTRAS veces para los diferentes valores de reintentos max. hasta MAXREINTENTOS
	for (uint32_t reintentos = 1; reintentos <= MAXREINTENTOS; reintentos++)
	{
		//Bucle para simular MUESTRAS veces para el mismo valor de reintentos
		for (uint32_t sim = 0; sim < MUESTRAS; sim++)
		{
			NS_LOG_INFO ("\nNumero de simulacion: " << sim + 1 << " Reintentos: " << reintentos);	

			simulacion(nCsma, delay, rate, packetSize, tstop, tstart, interval, perror,reintentos, &acumIntentos, &acumRetardos, &acumPaquetes);						
		}

		//Variables de error para los diferentes datos recolectados de las simulaciones
		double error1 = TSTUDENT * sqrt (acumIntentos.Var () / acumIntentos.Count ());
		datos_medias1.Add (reintentos, acumIntentos.Mean(), error1);

		double error2 = TSTUDENT * sqrt (acumRetardos.Var () / acumRetardos.Count ());
		datos_medias2.Add (reintentos, acumRetardos.Mean(), error2);

		double error3 = TSTUDENT * sqrt (acumPaquetes.Var () / acumPaquetes.Count ());
		datos_medias3.Add (reintentos, acumPaquetes.Mean(), error3);

		//Reseteamos los acumuladores para el siguiente valor de reintentos
		acumIntentos.Reset();
		acumRetardos.Reset();
		acumPaquetes.Reset();

	}

	// Declaro una Gráfica y la configuro
	Gnuplot Grafica01;
	Grafica01.SetLegend ("Reintentos", "Intentos");
	Grafica01.SetTitle ("Numero medio de intentos");
	// Añado la curva a la gráfica
	Grafica01.AddDataset (datos_medias1);
	// Para terminar genero el fichero
	std::ofstream fichero_01 ("practica03-01.plt");
	Grafica01.GenerateOutput (fichero_01);
	fichero_01 << "pause -1" << std::endl;
	fichero_01.close ();

	
	//Repetimos el proceso para generar las otras dos graficas
	Gnuplot Grafica02;
	Grafica02.SetLegend ("Reintentos", "Retardo(ms)");
	Grafica02.SetTitle ("Tiempo medio hasta recepcion del eco");
	Grafica02.AddDataset (datos_medias2);
	std::ofstream fichero_02 ("practica03-02.plt");
	Grafica02.GenerateOutput (fichero_02);
	fichero_02 << "pause -1" << std::endl;
	fichero_02.close ();

	Gnuplot Grafica03;
	Grafica03.SetLegend ("Reintentos", "Paquetes Correctos (%)");
	Grafica03.SetTitle ("Porcentaje de paquetes enviados correctamente");
	Grafica03.AddDataset (datos_medias3);
	std::ofstream fichero_03 ("practica03-03.plt");
	Grafica03.GenerateOutput (fichero_03);
	fichero_03 << "pause -1" << std::endl;
	fichero_03.close ();


	return 0;
}

//Funcion encargada de configurar el escenario y ejecutar la simulación.
void simulacion (uint32_t nCsma, Time delay, DataRate rate, uint32_t packetSize, Time tstop, Time tstart, std::string interval, double perror,
	uint32_t reintentos, Average<double> * acumIntentos, Average<double> * acumRetardos, Average<double> * acumPaquetes)
{
	// Aquí empieza el escenario
	NodeContainer csmaNodes;
	csmaNodes.Create (nCsma);
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", DataRateValue(rate));
	csma.SetChannelAttribute ("Delay", TimeValue (delay));
	Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
	errores->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
	errores->SetRate (perror);
	csma.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (errores));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (csmaNodes);



   // Instalamos la pila TCP/IP en todos los nodos
	InternetStackHelper stack;
	stack.Install (csmaNodes);
   // Y les asignamos direcciones
	Ipv4AddressHelper address;
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);
   // Cálculo de rutas
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   //Configuramos el tráfico
   /////////// Instalación de las aplicaciones

   // Servidor en el último nodo
	UdpEchoServerHelper echoServer (9);
	ApplicationContainer serverApp = echoServer.Install (csmaNodes.Get (nCsma - 1));
	serverApp.Start (Seconds(0.0));
	serverApp.Stop (tstop);
   // Clientes: en todos los nodos menos en el último
	UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma - 1), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
	echoClient.SetAttribute ("Interval", TimeValue (Time (interval)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
	NodeContainer clientes;
	for (uint32_t i = 0; i < nCsma - 1; i++)
		clientes.Add (csmaNodes.Get (i));
	ApplicationContainer clientApps = echoClient.Install (clientes);
	clientApps.Start (tstart);
	clientApps.Stop (tstop);


   //////////Aquí empieza la parte de análisis
   // Paquetes desde el punto de vista del servidor
	csma.EnablePcap ("p03_servidor", csmaDevices.Get (nCsma - 1), true);

   // Paquetes desde el punto de vista del cliente
   //for(uint32_t i = 0;i< nCsma-1; i++)
   //   csma.EnablePcap ("p03_cliente", csmaDevices.Get (i), true);


	Observador   servidor (csmaDevices.Get (nCsma -1)->GetObject<CsmaNetDevice> ());
	Observador * observadores[nCsma - 1];
	for (uint32_t i = 0; i < nCsma -1 ; i++)
	{
		observadores[i] = new Observador (csmaDevices.Get (i)->GetObject<CsmaNetDevice> ());
		(csmaDevices.Get(i)->GetObject<CsmaNetDevice> ())->SetBackoffParams (Time ("1us"), 10, 1000, 10 ,reintentos);
	}
	//Para servidor
	(csmaDevices.Get(nCsma-1)->GetObject<CsmaNetDevice> ())->SetBackoffParams (Time ("1us"), 10, 1000, 10 ,reintentos);


   // Lanzamos la simulacion
	Simulator::Run ();

   // Finalizamos el proceso de simulacion
	Simulator::Destroy ();

	//Variables para calcular la media de todos los nodos
	double sumaIntentos = 0;
	double sumaRetardos = 0;
	double sumaPerdidos = 0;
	//Variable auxiliar, para los casos en los que un nodo no es capaz de enviar los paquetes y el resultado es -Nan
	double tmp=0;
	
	//Bucle para obtener el numero medio de intentos de cada nodo
	for (uint32_t i = 0; i < nCsma-1; i++)
	{
		tmp = observadores[i]->GetIntentosMedio();

		//Comprobamos que tmp es un número mayor que cero
		if(tmp>0){
			NS_LOG_INFO ("Número medio de intentos de transmisión en el nodo " << i << ": " << observadores[i]->GetIntentosMedio());
			//Añadimos a los acumuladores los valores de tiempo medio y de retardo del nodo
			sumaIntentos += tmp;
			sumaRetardos += observadores[i]->GetRetardoMedio();	
		}
		else{
			//En el caso en el que no llega a enviarse ningun paquete, no añadimos el Nan a la suma para evitar errores
			NS_LOG_INFO ("Ha fallado la transmisión de todos los paquetes en el nodo " << i);

		}
	}
	//Restamos de la suma los valores del primer nodo (nodo 0)
	tmp = observadores[0]->GetIntentosMedio();
	if(tmp>0){
		sumaIntentos -= observadores[0]->GetIntentosMedio();
		sumaRetardos -= observadores[0]->GetRetardoMedio();
	}

	NS_LOG_INFO ("Número medio de intentos de transmisión en toda la topología: " << (sumaIntentos + servidor.GetIntentosMedio())/(nCsma -1));

	NS_LOG_INFO ("\nTiempo medio entre el envio de un paquete y la recepción del eco: " << sumaRetardos/((nCsma-2) * 10e6) << " ms\n");
	
	//Bucle para acumular los paquetes perdidos en el servidor
	//Realizamos esta suma en otro bucle diferente para que, al visualizar las trazas,
	//estas se generen en dos partes, por un lado el numero medio de intentos y por otro el porcentaje de paquetes perdidos
	for (uint32_t i = 0; i < nCsma-1; i++)
	{
		NS_LOG_INFO ("Porcentaje de paquetes perdidos en el nodo " << i << ": " << observadores[i]->GetPaqPerdidos());
		sumaPerdidos += observadores[i]->GetPaqPerdidos();
	}


	NS_LOG_INFO ("Porcentaje de paquetes perdidos en todos los clientes(%): " << sumaPerdidos/(nCsma -1));


	NS_LOG_INFO ("Porcentaje de paquetes perdidos en el servidor: " << servidor.GetPaqPerdidos());
	sumaPerdidos += servidor.GetPaqPerdidos();
	NS_LOG_INFO ("Porcentaje de paquetes perdidos en todos los nodos(%): " << sumaPerdidos/nCsma);	


	//Restamos los valores del nodo 0 y del servidor, ya que en el enunciado solo se especifican los clientes menos el primero
	sumaPerdidos -= (servidor.GetPaqPerdidos() + observadores[0]->GetPaqPerdidos());

	//Actualizamos los valores de los objetos Average para la generacion de graficas
	acumIntentos->Update(sumaIntentos/(nCsma - 2));
	acumPaquetes->Update(100 - sumaPerdidos/(nCsma - 2));
	acumRetardos->Update(sumaRetardos/((nCsma-2) * 10e6));

}