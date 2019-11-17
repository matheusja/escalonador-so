#include <vector>
#inclue "escalonador.hpp"
class escalonador::implementacao : public escalonador::interface {
public:
	implementacao(int cpus, int mem);
}



escalonador::escalonador(int cpus, int mem) : ptr_interface(new escalonador::implementacao(cpus, mem)) {} /* Initilizer list salvando a vida*/

escalonador &operator>>(std::istream &strm, escalonador &esc) {
	
}


escalonador &operator>>(escalonador &esc, std::ostream &strm) {
	
}