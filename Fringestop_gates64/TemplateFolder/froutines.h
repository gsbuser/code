void initialize_gibbs_sampler();

double gibbs_sample_gain(double *gain, double *vt, double *tt,
  double sigmamax, long n_ant, long n_iter);
