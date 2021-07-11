/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   Original work by John Wharington

*/


#ifndef WINDEKF_H
#define WINDEKF_H

unsigned WindKalmanUpdate(NMEA_INFO *basic, DERIVED_INFO *derived,  double *windspeed, double *windbearing );

class WindEKF {
  // constants/macros/typdefs

#define NUMX  3

  /** number of plant noise inputs, w is disturbance noise vector */
#define NUMW  3

  /** number of measurements, v is the measurement noise vector */
#define NUMV  1

  /** number of deterministic inputs, U is the input vector */
#define NUMU  2

  /// linearized system matrices
  float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];

  /// covariance matrix and state vector
  float P[NUMX][NUMX], X[NUMX];

  /// input noise and measurement noise variances
  float Q[NUMW], R[NUMV];

  /// feedback gain matrix
  float K[NUMX][NUMV];

public:
  void Init();
  void StatePrediction(float gps_vel[2], float dT);
  void Correction(float dynamic_pressure, float gps_vel[2]);


  /**
   * Does the prediction step of the Kalman filter for the covariance matrix
   */
  void CovariancePrediction(float dT);
  const float* get_state() const { return X; };

private:
  /**
   * Does the update step of the Kalman filter for the covariance and estimate
   */
  void SerialUpdate(float Z[NUMV], float Y[NUMV]);

  /**
   * Does a 4th order Runge Kutta numerical integration step
   */
  void RungeKutta(float U[NUMU], float dT);
  void StateEq(float U[NUMU], float Xdot[NUMX]);

  void LinearizeFG(float U[NUMU]);
  void MeasurementEq(float gps_vel[2], float Y[NUMV]);
  void LinearizeH(float gps_vel[2]);
};

#endif
