#include "binomial.h"

namespace Binomial {
// double OptionConfig::getBinomialValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
//   return (pu_ * futureValue + pd_ * currentValue)*exp(-riskFreeRate_*deltaT_);
// }
// 
// double OptionConfig::getSpotPrice(int currentStep, int numUpMovements) {
//   return S_ * pow(up_, 2*currentStep - (numUpMovements - 1));
// }

// QL European 
double QLEuropeanCall::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(getSpotPrice(currentStep, numUpMovements) - K_, 0.0);
}

double QLEuropeanCall::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  (void)currentStep;
  (void)numUpMovements;
  return getBinomialValue(currentValue, futureValue);
}

double QLEuropeanPut::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(K_ - getSpotPrice(currentStep, numUpMovements), 0.0);
}

double QLEuropeanPut::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  (void)currentStep;
  (void)numUpMovements;
  return getBinomialValue(currentValue, futureValue);
}

// QL American
double QLAmericanCall::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(getSpotPrice(currentStep, numUpMovements) - K_, 0.0);
}

double QLAmericanCall::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  return std::max(getExerciseValue(currentStep, numUpMovements), getBinomialValue(currentValue, futureValue));
}

double QLAmericanPut::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(K_ - getSpotPrice(currentStep, numUpMovements), 0.0);
}

double QLAmericanPut::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  return std::max(getExerciseValue(currentStep, numUpMovements), getBinomialValue(currentValue, futureValue));
}

// Zubair European
double ZubairEuropeanCall::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(getSpotPrice(currentStep, numUpMovements) - K_, 0.0);
}

double ZubairEuropeanCall::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  (void)currentStep;
  (void)numUpMovements;
  return getBinomialValue(currentValue, futureValue);
}

double ZubairEuropeanPut::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(K_ - getSpotPrice(currentStep, numUpMovements), 0.0);
}

double ZubairEuropeanPut::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  (void)currentStep;
  (void)numUpMovements;
  return getBinomialValue(currentValue, futureValue);
}

// Zubair American
double ZubairAmericanCall::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(getSpotPrice(currentStep, numUpMovements) - K_, 0.0);
}

double ZubairAmericanCall::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  return std::max(getExerciseValue(currentStep, numUpMovements), getBinomialValue(currentValue, futureValue));
}

double ZubairAmericanPut::getExerciseValue(int currentStep, int numUpMovements) {
  return std::max(K_ - getSpotPrice(currentStep, numUpMovements), 0.0);
}

double ZubairAmericanPut::getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  return std::max(getExerciseValue(currentStep, numUpMovements), getBinomialValue(currentValue, futureValue));
}

// ================= Stencil Computation ===================

void stencilTriangle(std::vector<double> &p, int blockSize, double pu) {
  for (int i = 0; i < blockSize-1; i++) {  // number of rows in triangle to look at
    for (int j = 0; j < blockSize-i; j++) {  // elts in that row
      p[j] = pu * p[j+1] + (1-pu) * p[j];
    }
  }
}

void stencilRhombus(std::vector<double> &p, int startIndex, int m1, int m2, double pu) {
  for (int i = 0; i < m1-1; i++) {
    for (int j = 0; j < m2; j++) {
      p[startIndex+j+m1-i-1] = pu * p[startIndex+j+m1-i] + 
        (1-pu) * p[startIndex+j+m1-i-1];
    }
  }
}

}  // namespace Binomial