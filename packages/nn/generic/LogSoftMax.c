#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/LogSoftMax.c"
#else

static int nn_(LogSoftMax_forward)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));
  real *input_data, *output_data;
  long nframe = 0, dim = 0;
  long t, d;

  if(input->nDimension == 1)
  {
    nframe = 1;
    dim = input->size[0];
  }
  else if(input->nDimension == 2)
  {
    nframe = input->size[0];
    dim = input->size[1];
  }
  else
    THArgCheck(0, 2, "vector or matrix expected");

  input = THTensor_(newContiguous)(input);
  THTensor_(resizeAs)(output, input);

  input_data = THTensor_(data)(input);
  output_data = THTensor_(data)(output);
  for(t = 0; t < nframe; t++)
  {
    accreal logsum = 0;
    real maxInput = -THInf;

    for(d = 0; d < dim; d++)
      maxInput = THMax(maxInput, input_data[d]);

    for(d = 0; d < dim; d++)
      logsum += THExpMinusApprox(maxInput-input_data[d]);
    logsum = maxInput + log(logsum);

    for(d = 0; d < dim; d++)
      output_data[d] = input_data[d] - logsum;

    input_data += dim;
    output_data += dim;
  }

  THTensor_(free)(input);

  return 1;
}

static int nn_(LogSoftMax_backward)(lua_State *L)
{
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_(Tensor_id));
  real *gradInput_data, *gradOutput_data, *output_data;
  long nframe = 0, dim = 0;
  long t, d;

  if(output->nDimension == 1)
  {
    nframe = 1;
    dim = output->size[0];
  }
  else if(output->nDimension == 2)
  {
    nframe = output->size[0];
    dim = output->size[1];
  }
  else
    THError("vector or matrix expected");

  THTensor_(resizeAs)(gradInput, output);
  gradInput_data = THTensor_(data)(gradInput);
  output_data = THTensor_(data)(output);
  gradOutput_data = THTensor_(data)(gradOutput);
  for(t = 0; t < nframe; t++)
  {
    accreal sum = 0;
    for(d = 0; d < dim; d++)
      sum += gradOutput_data[d];

    for(d = 0; d < dim; d++)
      gradInput_data[d] = gradOutput_data[d] - exp(output_data[d])*sum;

    gradInput_data += dim;
    output_data += dim;
    gradOutput_data += dim;
  }

  return 1;
}

static const struct luaL_Reg nn_(LogSoftMax__) [] = {
  {"LogSoftMax_forward", nn_(LogSoftMax_forward)},
  {"LogSoftMax_backward", nn_(LogSoftMax_backward)},
  {NULL, NULL}
};

void nn_(LogSoftMax_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(LogSoftMax__), "nn");
  lua_pop(L,1);
}

#endif
