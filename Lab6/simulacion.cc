/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Practica 06 - Simulación de Redes
Fecha: 05/12/17
Alumnos: Daniel Conde Ortiz
         Miguel Ruiz Ramos

Fichero: simulacion.cc
Descripcion: Este fichero contiene la funcion main y la funcion simulacion, encargadas de configurar el sistema, generar las graficas y ejecutar la simulacion
*/

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include <ns3/gnuplot.h>
#include <sstream>
#include "Enlace.h"
#include "ns3/error-model.h"
#include "ns3/average.h"

using namespace ns3;
#define RETARDO   "200us"
#define TASAE     "1Mb/s"
#define TASAFUENTE "500kb/s"
#define TON			"1s"
#define TOFF		"0s"
#define STOP      "25s"
#define RUN       "1s"
#define ERROR 0
#define MUESTRAS 10
#define TSTUDENT 1.833
#define TREENVIO "0.001s"
#define VENTANATX 1
#define VENTANARX 1

NS_LOG_COMPONENT_DEFINE ("Simulacion");

double simulacion(Time delay, DataRate ratee, DataRate ratef, Time tstop, Time tstart, Time ton, Time toff, double perror, uint32_t tam_paquete, uint32_t ventanatx, uint32_t ventanarx, Time timer);


int
main (int argc, char *argv[])
{
  // Ajustamos la resolución del reloj simulado a nanosegundos, por si se quiere disminuir el retardo del canal
	Time::SetResolution (Time::NS);

  // Parámetros de la simulación
	Time        delay (RETARDO);
	DataRate    ratee (TASAE);
	DataRate    ratef (TASAFUENTE);
	Time        tstop (STOP);
	Time        tstart (RUN);
	Time		ton (TON);
	Time 		toff (TOFF);
	double		perror = ERROR;
	Time 		timer(TREENVIO); //Valor por defecto pero puede ser cambiado
	uint32_t	tam_ventanatx = VENTANATX; //Valor por defecto, luego se cambiará
	uint32_t 	tam_ventanarx= VENTANARX;

	//Para configurar los valores de las variables mediante linea de comandos
	CommandLine cmd;

	cmd.AddValue ("delay", "Retardo de propagacion del enlace", delay);
	cmd.AddValue ("rate", "Capacidad de transmisión del canal", ratee);
	cmd.AddValue ("pErrorBit", "Probabilidad de error de bit", perror);
	cmd.AddValue ("tstart", "Tiempo de inicio de las aplicaciones", tstart);
	cmd.AddValue ("tstop", "Tiempo de fin de las aplicaciones", tstop);
	cmd.AddValue ("tam_ventanarx", "Tamaño de la ventana de recepcion", tam_ventanarx);
	cmd.AddValue ("timer", "Temporizador para el reenvio", timer);


	//Para añadir informacion sobre los parametros variables a la grafica
	std::ostringstream strs1;
	std::string sdelay;
	std::string sratee;
	std::string sperror;

	strs1 << delay.GetMicroSeconds();
	sdelay = strs1.str();
	strs1.str("");

	strs1 << ratee.GetBitRate()/1e3;
	sratee = strs1.str();
	strs1.str("");

	strs1 << perror;
	sperror = strs1.str();
	strs1.str("");

	//Comenzamos con la grafica
	Gnuplot Grafica;
	Grafica.SetLegend ("Tamanio del paquete (bytes)", "Rendimiento (%)");
	Grafica.SetTitle ("Rendimiento del envío segun el tamanio del paquete y de la ventana de envio:\\nRetardo del canal: "
		+ sdelay + "us; Capacidad de transimion del canal: " + sratee +
		+ "Kb/s\\nProbabilidad de error de bit: " + sperror);



	//Bucle para cada linea
	for(tam_ventanatx = 1; tam_ventanatx <= 5; tam_ventanatx++)
	{

		NS_LOG_INFO("\nComienzan las simulaciones para la curva con tamanio de ventana de envio " << tam_ventanatx);
		
		std::ostringstream strs2;
		std::string str2;

		strs2 << tam_ventanatx;
		str2 = strs2.str();

		//Creamos la linea
		Gnuplot2dDataset datos_medias("Tamanio de ventana de envio: " + str2 + " (paquetes)");
		
		datos_medias.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datos_medias.SetErrorBars (Gnuplot2dDataset::Y);
		

		//Bucle para cada tamaño de paquete
		for (uint32_t tam_paquete= 100; tam_paquete <= 25000; tam_paquete+=500)
		{
			Average<double> acumPorcent;
			
			NS_LOG_INFO("\nComienzan las simulaciones para el punto de tamanio de paquete " << tam_paquete << " bytes, se utiliza " << timer.GetMilliSeconds() << "ms como temporizador de retransmision");

			//Bucle para simular MUESTRAS veces para la misma probabilidad de error
			for (uint32_t sim = 0; sim < MUESTRAS; sim++)
			{
				NS_LOG_DEBUG("\nSimulacion: " << sim +1);

				double tmp;
				tmp = simulacion(delay, ratee, ratef, tstop, tstart, ton, toff, perror, tam_paquete, tam_ventanatx, tam_ventanarx, timer);					
				
				acumPorcent.Update(tmp*100);

			}

			//Variable de error para los diferentes datos recolectados de las simulaciones
			
			double error = TSTUDENT * sqrt (acumPorcent.Var () / acumPorcent.Count ());
			datos_medias.Add (tam_paquete, acumPorcent.Mean(), error);
			
		}

		// Añadimos la curva de rendimiento a las gráfica 2
		Grafica.AddDataset (datos_medias);
	}

	//Generamos el fichero con la grafica
	std::ofstream fichero ("practica06.plt");
	Grafica.GenerateOutput (fichero);
	fichero << "pause -1" << std::endl;
	fichero.close ();

	return 0;
}


double simulacion(Time delay, DataRate ratee, DataRate ratef, Time tstop, Time tstart, Time ton, Time toff, double perror, uint32_t tam_paquete, uint32_t ventanatx, uint32_t ventanarx, Time timer){

  ////// Componentes del escenario:

  // Creamos los nodos
	NodeContainer  nodos;
	nodos.Create(2);

 	//Creamos el escenario
	EnlaceHelper EnlHelp;

	EnlHelp.SetChannelAttribute("Delay",TimeValue (delay));
	EnlHelp.SetDeviceAttribute("DataRate", DataRateValue(ratee));


	//Error de bit
	Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
	errores->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
	errores->SetRate (perror);

	NetDeviceContainer nodosDevices;

	//Instalamos un Enlace en cada nodo y le pasamos los valores de ventana de transmision, recepcion y tiempo de reenvio
	//Tambien asignamos el modelo de error, con SetDeviceAtribute no funciona y hay que hacerlo a mano
	nodosDevices = EnlHelp.Install(nodos, ventanatx, ventanarx, timer, errores); 

	// Instalamos la pila TCP/IP en todos los nodos
	InternetStackHelper stack;
	stack.Install (nodos);
	
	// Y les asignamos direcciones
	Ipv4AddressHelper nodosAddress;
	nodosAddress.SetBase ("10.1.2.0", "255.255.255.0");

	Ipv4InterfaceContainer nodosInterfaces = nodosAddress.Assign (nodosDevices);

  	// Cálculo de rutas
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


	NS_LOG_DEBUG("Escenario configurado");

	// Preparamos las aplicaciones
	uint16_t sinkPort = 10000;

	//Una fuente y un sumidero en cada nodo

	//Helpers para los sumideros
	PacketSinkHelper sumidero0 ("ns3::UdpSocketFactory",InetSocketAddress(nodosInterfaces.GetAddress(0),sinkPort));
	PacketSinkHelper sumidero1 ("ns3::UdpSocketFactory",InetSocketAddress(nodosInterfaces.GetAddress(1),sinkPort));

	//Helpers para las fuentes
	OnOffHelper fuente0("ns3::UdpSocketFactory",InetSocketAddress(nodosInterfaces.GetAddress(0),sinkPort));
	OnOffHelper fuente1("ns3::UdpSocketFactory",InetSocketAddress(nodosInterfaces.GetAddress(1),sinkPort));

	//Obtenemos los valores de ton y toff que van a tener las fuentes
	std::ostringstream strs1;
	std::ostringstream strs2;
	std::string str;

	strs1 << ton.GetSeconds();
	str = strs1.str();

	std::string ontime = "ns3::ConstantRandomVariable[Constant="+ str +"]";
	
	strs2 << toff.GetSeconds();
	str = strs2.str();

	std::string offtime = "ns3::ConstantRandomVariable[Constant="+ str +"]";

	//Se los asignamos, junto a otros atributos
	fuente0.SetAttribute("OnTime", StringValue(ontime));
	fuente0.SetAttribute("OffTime", StringValue(offtime));
	fuente0.SetAttribute("DataRate", DataRateValue(ratef));
	fuente0.SetAttribute("PacketSize", UintegerValue(tam_paquete));

	fuente1.SetAttribute("OnTime", StringValue(ontime));
	fuente1.SetAttribute("OffTime", StringValue(offtime));
	fuente1.SetAttribute("DataRate", DataRateValue(ratef));
	fuente1.SetAttribute("PacketSize", UintegerValue(tam_paquete));

	//Instalamos las aplicaciones
	ApplicationContainer fuentesApp = fuente0.Install(nodos.Get(1));
	fuentesApp.Add(fuente1.Install(nodos.Get(0)));

	fuentesApp.Start(tstart);
	fuentesApp.Stop(tstop);

	ApplicationContainer sumideroApp = sumidero0.Install(nodos.Get(0));
	sumideroApp.Add(sumidero1.Install(nodos.Get(1)));

	sumideroApp.Start(tstart);
	
	//EnlHelp.EnablePcap ("p6", nodosDevices.Get (1), true);
	//EnlHelp.EnablePcap ("p6", nodosDevices.Get (0), true);

	Simulator::Run ();
	Simulator::Destroy ();

	//Obtenemos el tiempo de espera de cada nodo, hacemos la media y calculamos el porcentaje del tiempo total
	double total = ((nodosDevices.Get (0)->GetObject<Enlace> ())->GetDatos() + (nodosDevices.Get (1)->GetObject<Enlace> ())->GetDatos())/2;
	double total_posible = ratef.GetBitRate() *(tstop-tstart).GetSeconds();
	NS_LOG_DEBUG("Porcentaje: " << total/total_posible);

	return total/total_posible;
}