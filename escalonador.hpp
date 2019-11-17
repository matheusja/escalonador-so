#ifndef ESCALONADOR_HPP_INCLUDED
#define ESCALONADOR_HPP_INCLUDED
#include <memory> /* std::unique_ptr*/
#include <iosfwd> /* std::{i, o}stream (&)*/
class escalonador {
public:
	class resultado;
	class interface {
	public:
		friend escalonador &operator>>(std::istream &strm, escalonador &esc);
		friend escalonador &operator>>(escalonador &esc, std::ostream &strm);
		virtual  interface(int cpus, int mem) = 0;
		virtual ~interface() = default; /* método destruidor virtual para permitir a deleção adequada no caso de polimorfismo*/
		virtual void read_stream(std::istream &strm);
		virtual void writ_stream(std::ostream &strm);
	};
	escalonador(int cpus, int mem);
	/* destruidor implicito suficiente*/
private:
	std::unique_ptr<escalonador::interface> ptr_interface
	/* sera utilizado depois */
	class implementacao;
};
/**
	Ler o stream e realizar a simulação
*/
resultado &operator>>(std::istream &strm, escalonador &esc);
/**
	Imprimir o resultado da simulação
*/
escalonador &operator>>(escalonador &esc, std::ostream &strm);

#endif//ESCALONADOR_HPP_INCLUDED

