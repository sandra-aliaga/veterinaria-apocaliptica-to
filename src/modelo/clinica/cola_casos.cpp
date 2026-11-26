#include "cola_casos.h"

#include <utility>

namespace veterinaria {

struct ColaCasosClinicos::Nodo {
    explicit Nodo(CasoClinico ficha) : caso(std::move(ficha)) {
    }

    CasoClinico caso;
    std::unique_ptr<Nodo> siguiente;
};

ColaCasosClinicos::ColaCasosClinicos() : ultimo_(nullptr) {
}

// Libera los nodos de uno en uno para no desbordar la pila con colas largas.
ColaCasosClinicos::~ColaCasosClinicos() {
    while (primero_) {
        auto retirado = std::move(primero_);
        primero_ = std::move(retirado->siguiente);
    }
    ultimo_ = nullptr;
}

void ColaCasosClinicos::encolar(CasoClinico caso) {
    auto nodo = std::make_unique<Nodo>(std::move(caso));
    Nodo* const nuevo = nodo.get();
    if (ultimo_ == nullptr) {
        primero_ = std::move(nodo);
    }

    else {
        ultimo_->siguiente = std::move(nodo);
    }
    ultimo_ = nuevo;
}

std::optional<CasoClinico> ColaCasosClinicos::extraer() {
    if (!primero_) {
        return std::nullopt;
    }
    std::unique_ptr<Nodo> retirado = std::move(primero_);
    primero_ = std::move(retirado->siguiente);
    if (!primero_) {
        ultimo_ = nullptr;
    }
    return std::move(retirado->caso);
}

const CasoClinico* ColaCasosClinicos::frente() const {
    return primero_ ? &primero_->caso : nullptr;
}

} // namespace veterinaria
