#include <iostream> /* std::cin, std::cout*/
#include <vector>  /* std::vector */
#include <cstddef> /* std::size_t*/

struct process_on_queue {
    const static int TIME_UNTIL_PROMOTION = 10;
    enum class status {
        change_priority, terminate, wait
    };
    int prioridade_inicial;
    int tempo_restante;
    int memoria;
    int time_in_queue;
    bool ascending;
    status operator++() {
        this->tempo_restante--;
        if (this->tempo_restante == 0) {
            return status::terminate;
        }
        time_in_queue++;
        if (times_in_queue == TIME_UNTIL_PROMOTION) {
            return status::change_priority;
        }
        return status::wait;
    };
};

class queue_iterator { /* iterador para a fila, um forward iterator */
private:
    std::vector<process> *ref_fila;
    std::size_t index;
public: 
    queue_iterator(std::vector<process> *ptr, std::size_t index) : ref_fila(ptr), index(0) {}; /* caso base, as outras sao baseados nesse */
    
    queue_iterator()                             : fila_iterator(null_ptr        , 0) {};
    queue_iterator(std::vector<process> &ptr)    : fila_iterator(&ptr            , 0) {};
    queue_iterator(std::vector<process> *ptr)    : fila_iterator(ptr             , 0) {};
    queue_iterator(const fila_iterator &copy)    : fila_iterator(copy.ref_fila   , copy.index) {};
    queue_iterator operator=(const fila_iterator &copy) {
        this->ref_fila = copy.ref_fila;
        this->index = copy.index;
    }
    void operator++(int) {
        this->index = (this->index + 1) % ref_fila->size();
    };
    void operator++() {
        index = (index + 1) % ref_fila->size();
    };
    process &operator*() {
        return this->ref_fila->at(this->index);
    }
};

class 

int main(int argc, char **argv) {
    return 0;
}
