#include "arma_base.h"

#include <algorithm>

namespace veterinaria {

ArmaBase::ArmaBase(int danio, float intervalo_entre_ataques)
    : danio_(std::max(0, danio)), intervalo_entre_ataques_(std::max(0.0F, intervalo_entre_ataques)),
      segundos_para_atacar_(0.0F) {
}

void ArmaBase::reiniciar() {
    // Cada intento empieza con el arma lista.
    segundos_para_atacar_ = 0.0F;
}

void ArmaBase::actualizar(float segundos) {
    if (segundos <= 0.0F) {
        return;
    }

    segundos_para_atacar_ = std::max(0.0F, segundos_para_atacar_ - segundos);
}

float ArmaBase::alcance() const {
    // El arma basica golpea solo al contacto.
    return 0.0F;
}

int ArmaBase::intentar_atacar() {
    if (segundos_para_atacar_ > 0.0F) {
        return 0;
    }

    segundos_para_atacar_ = intervalo_entre_ataques_;
    return danio_;
}

} // namespace veterinaria
