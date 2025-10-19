#include "../include/PointsManager.hpp"

using namespace geode::prelude;
/*
void PointsManager::setLastDeath(float lastDeath) {
    this->m_lastDeath = lastDeath;
    this->updatePoints();
}
float PointsManager::getLastDeath() { return this->m_lastDeath; }
void PointsManager::setLabel(CCLabelBMFont* label) { this->m_label = label; }
CCLabelBMFont* PointsManager::getLabel() { return this->m_label; }
float PointsManager::getPoints() { return this->m_points; }
void PointsManager::setPoints(float points) { this->m_points = points; }
void PointsManager::addPoints(float points) { this->m_points += points; }

void PointsManager::updatePoints() {
    float deathPercent = this->getLastDeath();

    if (deathPercent < 40)
        return;

    if (deathPercent < 56) {
        this->addPoints(deathPercent);
        return;
    }

    if (deathPercent < 80) {
        this->addPoints(deathPercent * 1.5f);
        return;
    }

    this->addPoints(deathPercent * 2.f);
}*/