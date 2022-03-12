/*
Practica 05 - Simulación de Redes
Fecha: 02/12/17
Alumnos: Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: Observador.cc
Descripcion: Este fichero contiene el código de las clase Observador, necesarias para observar el escenario de la práctica 5 y obtener los
			 resultados estadisticos.
*/

#include "Observador.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

//Clase Observador para obtener los valores necesitados de cada uno de los nodos
Observador::Observador (Ptr<CsmaNetDevice> csmaDevicefuente, Ptr<PointToPointNetDevice> device_sumidero, ApplicationContainer * sumideroapp, ApplicationContainer * fuentes, uint32_t flag_grafica, Time tstop)
{	

	NS_LOG_INFO("Observador creado");

	//Asignamos valores a las variables
	m_csmaDevicefuente = csmaDevicefuente;
	m_sumidero_ptr=device_sumidero;	
	m_sumideroapp = sumideroapp;
	m_fuentes = fuentes;
	m_flag_grafica = flag_grafica;
	octetosRx=0;
	recibidos=0;
	m_tstop = tstop;

	//Curva donde vamos a ir guardando el tamaño de la ventana de congestion
	CongWin = new Gnuplot2dDataset();

	//Llamamos a las funciones necesarias para poner en funcionamiento el observador
	ProgramaTrazas ();
	ultimo = Simulator::Now();	
	Simulator::Schedule(Seconds(3), &Observador::Comprueba, this);
}

/*
Metodo: ProgramaTrazas
Descripcion: Se colocan las trazas que se quieren observar y se asigna a cada una de ellas la función de callback a la que deberán llamar cuando ocurran.
*/
void
Observador::ProgramaTrazas ()
{

	m_sumidero_ptr->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&Observador::PaqueteSube, this));
}	
/*
Metodo: TrazaSocket
Descripcion: Obtiene el objeto del socket de la fuente y le programa una traza para vigilar la ventana de congestion.
			 Se ejecuta una vez se ha enviado el primer paquete, correspondiente al SYN de la conexion.
*/
void Observador::TrazaSocket()
{
	socketFuente = m_fuentes->Get(0)->GetObject<OnOffApplication>()->GetSocket()->GetObject<TcpSocketBase>();
	socketFuente->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&Observador::GetCongestion, this));
}

/*
Metodo: GetCongestion
Descripcion:Introduce los datos de la ventana de congestion en el dataset para posteriormente representar las graficas

*/
void Observador::GetCongestion(uint32_t old, uint32_t newo)
{
	if(m_flag_grafica == 1)
		CongWin->Add (Simulator::Now().GetMilliSeconds(), newo);
}

/*
Metodo: GetDatos
Descripcion: Devuelve el puntero al objeto que contiene los datos para la grafica de la ventana de congestion
*/
Gnuplot2dDataset * Observador::GetDatos()
{
	return  CongWin;
}

/*
Metodo: PaqueteSube
Descripcion:Funcion callback para cuando se recibe un paquete.
			Cuando se recibe el primer paquete llama a TrazaSocket.
			Cada vez que se recibe un paquete actualiza el valor de la variable ultimo y
			suma el tamaño del paquete a octetosRx
*/
void Observador::PaqueteSube (Ptr<const Packet> pqt)
{
	if(recibidos == 0)
		this->TrazaSocket();
	ultimo = Simulator::Now();	
	octetosRx+=pqt->GetSize();
	recibidos++;
	NS_LOG_INFO("Recibido " << Simulator::Now().GetSeconds());
}


/*
Metodo: Comprueba
Descripcion: Esta funcion nos sirve para comprobar si han pasado X (por defecto 2) segundos
			 desde la última vez que se recibió un paquete. Si es así, y ha pasado el tiempo de stop
			 de las fuentes se para el sumidero; si no, se programa la función para dentro de 2 segundos.
*/
void Observador::Comprueba()
{

	if((Simulator::Now()).GetSeconds() - ultimo.GetSeconds() > 2 && Simulator::Now() > m_tstop)
	{
		m_sumideroapp->Stop(Simulator::Now() + MilliSeconds(10));
		NS_LOG_DEBUG("Programado el apagado del sumidero");
		m_sumidero_ptr->TraceDisconnectWithoutContext("MacRx", MakeCallback (&Observador::PaqueteSube, this));
	}
	else
	{
		Simulator::Schedule(Seconds(2), &Observador::Comprueba, this);
	}
}

/*
Metodo: GetOctetosRecibidos
Descripcion: Devuelve el numero de octetos recibidos correctamente en el sumidero
*/

double Observador::GetOctetosRecibidos(){
	return octetosRx;

}