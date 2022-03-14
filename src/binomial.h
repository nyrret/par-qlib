#include <cstdint>
#include <iostream>
#include <math.h>
#include <vector>

#include "gettime.h"

#include <parlay/parallel.h>
#include <parlay/sequence.h>

namespace Binomial {
class OptionConfig {
  public:
    explicit OptionConfig(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate)
    : steps_{steps}, deltaT_{deltaT}, S_{S}, K_{K}, riskFreeRate_{riskFreeRate} {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);

    inline double getBinomialValue(double currentValue, double futureValue) {
      return (pu_ * futureValue + pd_ * currentValue)*exp(-riskFreeRate_*deltaT_);
    }
    inline double getSpotPrice(int currentStep, int numUpMovements) {
      return S_ * pow(up_, 2*currentStep - (numUpMovements - 1));
    }

  public: // TODO
    int steps_;
    double deltaT_;
    double S_;  // initial price
    double K_;  // strike price
    double riskFreeRate_;
    double pu_;
    double pd_;
    double up_;
};

static inline __attribute__((always_inline)) double getBinomialValueHelper(
  double currentValue, 
  double futureValue,
  double pu,
  double pd,
  double riskFreeRate,
  double deltaT
) {
  return (pu * futureValue + pd * currentValue)*exp(-riskFreeRate*deltaT);
}

static inline __attribute__((always_inline)) double getSpotPriceHelper(
  int currentStep, 
  int numUpMovements,
  double S,
  double up
) {
  return S * pow(up, 2*currentStep - (numUpMovements - 1));
}

// ==========QuantLib=======================

class QuantLibConfig : public OptionConfig {
  public:
    explicit QuantLibConfig(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : OptionConfig(steps, deltaT, S, K, riskFreeRate) {
      double dx = volatility * sqrt(deltaT);
      double drift_per_step = (riskFreeRate - dividendYield - 0.5 * volatility * volatility) * deltaT;
      pu_ = 0.5 + 0.5 * drift_per_step / dx;
      pd_ = 1-pu_;

      up_ = exp(volatility * sqrt(deltaT));
    }
};

class QLEuropeanCall: public QuantLibConfig { // TODO inherits
  public: 
    explicit QLEuropeanCall(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : QuantLibConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}
    // TODO replace constructor body with above line
    // ) : steps_{steps}, S_{S}, K_{K}, deltaT_{deltaT}, riskFreeRate_{riskFreeRate} {
    //   double dx = volatility * sqrt(deltaT);
    //   double drift_per_step = (riskFreeRate - dividendYield - 0.5 * volatility * volatility) * deltaT;
    //   pu_ = 0.5 + 0.5 * drift_per_step / dx;
    //   pd_ = 1-pu_;

    //   up_ = exp(volatility * sqrt(deltaT));
    // }

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
  //   inline __attribute__((always_inline)) double getExerciseValue(int currentStep, int numUpMovements) {
  //     return std::max(S_ * pow(up_, 2*currentStep - (numUpMovements - 1)) - K_, 0.0);
  //     // return std::max(getSpotPrice(currentStep, numUpMovements) - K_, 0.0);
  //   }
  //   inline __attribute__((always_inline)) double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  //     // return getBinomialValue(currentValue, futureValue, currentStep, numUpMovements);
  //     return (pu_ * futureValue + pd_ * currentValue)*exp(-riskFreeRate_*deltaT_);
  //   }

  //   // TODO: everything below here is from OptionConfig
  //   inline __attribute__((always_inline)) double getBinomialValue(double currentValue, double futureValue, int currentStep, int numUpMovements) {
  //     return (pu_ * futureValue + pd_ * currentValue)*exp(-riskFreeRate_*deltaT_);
  //     // return getBinomialValueHelper(
  //     //   currentValue,
  //     //   futureValue,
  //     //   currentStep,
  //     //   numUpMovements,
  //     //   pu_,
  //     //   pd_,
  //     //   riskFreeRate_,
  //     //   deltaT_
  //     // );
  //   }
  //   inline __attribute__((always_inline)) double getSpotPrice(int currentStep, int numUpMovements) {
  //     return S_ * pow(up_, 2*currentStep - (numUpMovements - 1));
  //     // return getSpotPriceHelper(currentStep, numUpMovements, S_, up_);
  //   }

  // public: // TODO: protected
  //   int steps_;
  //   double deltaT_;
  //   double S_;  // initial price
  //   double K_;  // strike price
  //   double riskFreeRate_;
  //   double pu_;
  //   double pd_;
  //   double up_;
};

class QLEuropeanPut: public QuantLibConfig {
  public: 
    explicit QLEuropeanPut(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : QuantLibConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

class QLAmericanCall: public QuantLibConfig {
  public: 
    explicit QLAmericanCall(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : QuantLibConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

class QLAmericanPut: public QuantLibConfig {
  public: 
    explicit QLAmericanPut(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : QuantLibConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

// ==============Zubair=======================

class ZubairConfig : public OptionConfig {
  public: 
    explicit ZubairConfig(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : OptionConfig(steps, deltaT, S, K, riskFreeRate) {
      up_ = exp(volatility * sqrt(deltaT));
      double down = 1.0/up_;

      pu_ = (exp((riskFreeRate-dividendYield)*deltaT) - down)/(up_-down);
      pd_ = 1-pu_;
    }
};

class ZubairEuropeanCall: public ZubairConfig {
  public: 
    explicit ZubairEuropeanCall(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : ZubairConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

class ZubairEuropeanPut: public ZubairConfig {
  public: 
    explicit ZubairEuropeanPut(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : ZubairConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

class ZubairAmericanCall: public ZubairConfig {
  public: 
    explicit ZubairAmericanCall(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : ZubairConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

class ZubairAmericanPut: public ZubairConfig {
  public: 
    explicit ZubairAmericanPut(
      int steps,
      double deltaT,
      double S,
      double K,
      double riskFreeRate,
      double volatility,
      double dividendYield
    ) : ZubairConfig(steps, deltaT, S, K, riskFreeRate, volatility, dividendYield) {}

    double getExerciseValue(int currentStep, int numUpMovements);
    double getNodeValue(double currentValue, double futureValue, int currentStep, int numUpMovements);
};

template <class Config>
double binomialTraversal(int steps, int expirationTime, double S, double K, double riskFreeRate, double volatility, double dividendYield = 0) {
  // TODO: put this back in if use inheritance
  // static_assert(std::is_base_of<OptionConfig, Config>::value,
  //   "Config must be a derived class of OptionConfig");

  // Timer t = Timer{};
  // t.start();
  double deltaT = (double)expirationTime/steps/365;

  Config config = Config{steps, deltaT, S, K, riskFreeRate, volatility, dividendYield};
  // t.reportNext("Init Config");

  // initial values at expiration time
  std::vector<double> p;
  for (int i = 0; i < steps+1; ++i) {
    p.push_back(config.getExerciseValue(i, steps+1)); 
    if (p[i] < 0) {
      p[i] = 0;
    }
  }
  // t.reportNext("Init values at expiration time");

  // move to earlier times 
  for (int j = steps; j >= 0; --j) {
    for (int i = 0; i < j; ++i) {
      // binomial value
      p[i] = config.getNodeValue(p[i], p[i+1], i, j);
    }
  }
  // t.reportNext("Compute binomial values");

  return p[0];
}

template <class Config>
double parallelBinomialTraversal(int steps, int expirationTime, double S, double K, double riskFreeRate, double volatility, double dividendYield = 0) {
  // TODO: put back in if use inheritance
  // static_assert(std::is_base_of<OptionConfig, Config>::value,
  //   "Config must be a derived class of OptionConfig");

  double deltaT = (double)expirationTime/steps/365;

  Config config = Config{steps, deltaT, S, K, riskFreeRate, volatility, dividendYield};

  // initial values at expiration time
  std::vector<double> p;
  for (int i = 0; i < steps+1; ++i) {
    p.push_back(config.getExerciseValue(i, steps+1)); 
    if (p[i] < 0) {
      p[i] = 0;
    }
  }

  // move to earlier times 
  std::vector<double> pastValues(steps+1);
  p.swap(pastValues);
  for (int j = steps; j >= 0; --j) {
    parlay::parallel_for(0, j, [&](int i) {
      // binomial value
      p[i] = config.getNodeValue(pastValues[i], pastValues[i+1], i, j);
    });
    p.swap(pastValues);
  }

  return p[0];
}

// ==========================================================
// =============Stencil Computation==========================
// ==========================================================

// level -- how many levels deep bottom of this triangle is
//  (top point in entire stencil computation is at level 1)
template <class Config>
void stencilTriangle(
  // std::vector<double> &p, int m1, int blockSize, bool isOnBottom, int numTrianglesAbove, Config &config) {
  std::vector<double> &p, int m1, bool isOnBottom, int level, Config &config) {
  // for (int i = 0; i < blockSize-1; i++) {  // number of rows in triangle to look at
  //   for (int j = 0; j < blockSize-i; j++) {  // elts in that row
  for (int i = 0; i < m1-1; i++) {  // number of rows in triangle to look at
    for (int j = 0; j < m1-i-1; j++) {  // elts in that row
  // for (int i = blockSize-2; i >= 0; --i) { 
  //   for (int j = 0; j < i+2; ++j) {
      // p[j] = config.getNodeValue(p[j], p[j+1], j, blockSize-1-i);
      p[j] = config.getNodeValue(
        p[j], p[j+1], j, level-isOnBottom-i);
        // p[j], p[j+1], j, (numTrianglesAbove+1)*blockSize-isOnBottom-i);
    }
  }
}

// level -- how many levels deep bottom of this rhombus is
//  (top point in entire stencil computation is at level 1)
template <class Config>
void stencilRhombus(
  std::vector<double> &p, 
  int startIndex, 
  int m1, 
  int m2, 
  int level,
  bool isOnBottom,
  Config &config
) {
  for (int i = 0; i < m1-1; i++) {
    for (int j = 0; j < m2; j++) {
  // for (int i = m1-1; i >= 0; --i) {
  //   for (int j = 0; j < m2; ++j) {
      p[startIndex+j+m1-i-2] = config.getNodeValue(
        p[startIndex+j+m1-i-2], 
        p[startIndex+m1+j-i-1], 
        startIndex+j+m1-i-2, // startIndex+j, 
        level-isOnBottom-i // startIndex+m1-2-i
        // (numRhombusesAbove+2)*m2-isOnBottom-i // startIndex+m1-2-i
      );
    }
  }
}

template <class Config>
double stencilBinomialTraversal(int steps, int expirationTime, double S, double K, double riskFreeRate, double volatility, double dividendYield = 0) {
  // TODO: put this back in if use inheritance
  // static_assert(std::is_base_of<OptionConfig, Config>::value,
  //   "Config must be a derived class of OptionConfig");

  double deltaT = (double)expirationTime/steps/365;

  Config config = Config{steps, deltaT, S, K, riskFreeRate, volatility, dividendYield};

  // initial values at expiration time
  std::vector<double> p;
  for (int i = 0; i < steps+1; ++i) {
    p.push_back(config.getExerciseValue(i, steps+1));
    if (p[i] < 0) {
      p[i] = 0;
    }
  }

  // stencil computation
  // TODO -- compile-time constant, can take param at compile-time
  const int cacheCapacity = 7;   
  const int blockSize = (cacheCapacity+1)/2; 
  const int numBlocks = (steps+1)/blockSize; 
  const int edgeBlockSize = (steps+1)%blockSize;

  stencilTriangle(p, blockSize, true, numBlocks*blockSize + edgeBlockSize, config);
  for (int i = 1; i < numBlocks; i++) {
    stencilRhombus(
      p, 
      (i-1)*blockSize + 1, 
      blockSize, 
      blockSize, 
      numBlocks*blockSize + edgeBlockSize,
      true,
      config
    );
    for (int j = 1; j <= i-1; j++) {
      stencilRhombus(
        p, 
        (i-j-1)*blockSize + 1, 
        blockSize+1, 
        blockSize, 
        (numBlocks-j)*blockSize + edgeBlockSize,
        false,
        config
      );
    }
    stencilTriangle(
      p, blockSize+1, false, (numBlocks-i)*blockSize + edgeBlockSize, config);
  }

  // extra blocks in case did not divide evenly
  if (edgeBlockSize > 0) {
    stencilRhombus(
      p, 
      (numBlocks-1)*blockSize + 1, 
      blockSize, 
      edgeBlockSize, 
      numBlocks*blockSize + edgeBlockSize, 
      true, 
      config
    );
    for (int i = 1; i < numBlocks; i++) {
      stencilRhombus(
        p, 
        (numBlocks-i-1)*blockSize + 1, 
        blockSize+1, 
        edgeBlockSize, 
        (numBlocks-i)*blockSize + edgeBlockSize, 
        false, 
        config
      );
    }
    stencilTriangle(p, edgeBlockSize+1, false, edgeBlockSize, config);
  }

  return p[0];
}  // end stencilBinomialTraversal

// =================================================
// ===============Parallel Stencil==================
// =================================================

// level -- how many levels deep bottom of this triangle is
//  (top point in entire stencil computation is at level 1)
template <class Config>
void parallelStencilTriangle(
  // std::vector<double> &p, int m1, int blockSize, bool isOnBottom, int numTrianglesAbove, Config &config) {
  std::vector<double> &p, int m1, int level, Config &config) {
  // for (int i = 0; i < blockSize-1; i++) {  // number of rows in triangle to look at
  //   for (int j = 0; j < blockSize-i; j++) {  // elts in that row
  for (int i = 0; i < m1-1; i++) {  // number of rows in triangle to look at
    for (int j = 0; j < m1-i-1; j++) {  // elts in that row
  // for (int i = blockSize-2; i >= 0; --i) { 
  //   for (int j = 0; j < i+2; ++j) {
      // p[j] = config.getNodeValue(p[j], p[j+1], j, blockSize-1-i);
      p[j] = config.getNodeValue(
        p[j], p[j+1], j, level-1-i);  // -1 because all the triangles are on the bottom
    }
  }
}

// level -- how many levels deep the left bottom corner of this rhombus is
//  (top point in entire stencil computation is at level 1)
template <class Config>
void parallelStencilRhombus(
  std::vector<double> &p, 
  std::vector<double> &pastValues,
  int startIndex, 
  int m1, 
  int level,
  Config &config
) {
  // up until middle row
  for (int i = 0; i < m1; i++) {  // row 
    for (int j = 0; j < i+1; j++) {  // column
      // start at bottom right point, and go up to middle row
      p[startIndex+(m1-i-1)+j] = config.getNodeValue(
        pastValues[startIndex+(m1-i-1)+j],   // current value
        pastValues[startIndex+(m1-i-1)+j+1],   // future value
        startIndex+(m1-i-1)+j, 
        level+(m1-i-1) 
      );
    }
  }
  // up above middle row until top point
  for (int i = 0; i < m1-1; i++) {  // row
    for (int j = 0; j < m1-i-1; j++) {  // column
      p[startIndex+j] = config.getNodeValue(
        pastValues[startIndex+j],   // current value
        pastValues[startIndex+j+1],   // future value
        startIndex+j,
        level-i-1   
      );
    }
  }
}

template <class Config>
double parallelStencilBinomialTraversal(int steps, int expirationTime, double S, double K, double riskFreeRate, double volatility, double dividendYield = 0) {
  // TODO: put this back in if use inheritance
  // static_assert(std::is_base_of<OptionConfig, Config>::value,
  //   "Config must be a derived class of OptionConfig");

  double deltaT = (double)expirationTime/steps/365;

  Config config = Config{steps, deltaT, S, K, riskFreeRate, volatility, dividendYield};

  // initial values at expiration time
  std::vector<double> p;
  for (int i = 0; i < steps+1; ++i) {
    p.push_back(config.getExerciseValue(i, steps+1));
    if (p[i] < 0) {
      p[i] = 0;
    }
  }

  // stencil computation
  // TODO -- compile-time constant, can take param at compile-time
  const int cacheCapacity = 7;   
  const int blockSize = (cacheCapacity+1)/2; 
  const int numBlocks = (steps+1)/blockSize; 
  const int edgeBlockSize = (steps+1)%blockSize;

  // compute bottom row of triangles
  // TODO -- parallelize this? 
  for (int i = 0; i < numBlocks; i++) {
    parallelStencilTriangle(p, blockSize, numBlocks*blockSize + edgeBlockSize, config);
  }

  // TODO -- finish the computation for normal sizes
  // TODO -- make this actually parallel
  std::vector<double> pastValues(steps+1);
  for (int i = 0; i < numBlocks; i++) {
    // p.swap(pastValues); -- TODO
    // compute rhombuses in a row in the same loop
    // (bottom to top)
    // TODO: make actually parallel
    // parlay::parallel_for(0, numBlocks-i, [&](int j) {
    for (int j = 0; j < numBlocks-i+1; j++) {
      parallelStencilRhombus(
        p, 
        p,
        i*blockSize,
        blockSize,
        // TODO -- I'm guessing this is screwed up
        (numBlocks-j-1)*blockSize + edgeBlockSize,
        config
      );
    }
    // });
  }

  // TODO -- finish for edge blocks

  return p[0];
}  // end parallelStencilBinomialTraversal
}  // namespace Binomial
