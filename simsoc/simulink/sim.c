#include "sim.h"
#include "soc.h"
 
int in[INPUT_COUNT];
out_type out;

system_param_type  SystemParam;
sys_flg_type       SystemFlg;
sys_tick_type      SystemTick;
soc_count_type     SocCounter;
block_type         BlockParam;
battype            Bat;
soctype            Soc;
captype            Cap;
histype            His;
soetype            Soe;
markstype          Marks;
soc_integral_type  Integral;

void RunAlgorithm(void)
{  
    maintask(in[0], in[1], in[2]/8, 0, 25, 13.3*64);
}
