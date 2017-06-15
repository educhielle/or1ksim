/* insnset.c -- Instruction specific functions.

   Copyright (C) 1999 Damjan Lampret, lampret@opencores.org
                 2000-2002 Marko Mlinar, markom@opencores.org
   Copyright (C) 2008 Embecosm Limited
   Copyright (C) 2009 Jungsook yang, jungsook.yang@uci.edu
  
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
   Contributor Julius Baxter julius@orsoc.se  

   This file is part of OpenRISC 1000 Architectural Simulator.
  
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your option)
   any later version.
  
   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.
  
   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* This program is commented throughout in a fashion suitable for processing
   with Doxygen. */

/** MoMA begin **/
//#include <gmp.h> // included in ./cpu/or1k/arch.h
/** MoMA end **/

INSTRUCTION (l_add) {
  orreg_t temp1, temp2, temp3;
  int8_t temp4;
  
  temp2 = (orreg_t)PARAM2;
  temp3 = (orreg_t)PARAM1;
  temp1 = temp2 + temp3;
  SET_PARAM0 (temp1);

  /* Set overflow if two negative values gave a positive sum, or if two
     positive values gave a negative sum. Otherwise clear it */
  if ((((long int) temp2 <  0) && 
       ((long int) temp3 <  0) &&
       ((long int) temp1 >= 0)) ||
      (((long int) temp2 >= 0) && 
       ((long int) temp3 >= 0) &&
       ((long int) temp1 <  0)))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* Set the carry flag if (as unsigned values) the result is smaller than
     either operand (if it smaller than one, it will be smaller than both, so
     we need only test one). */
  if ((uorreg_t) temp1 < (uorreg_t) temp2)
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_CY;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_CY;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for either CY or OV, and set
         appropriate AESR bit, then raise exception */
      if ((((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_CYADDE) == SPR_AECR_CYADDE)) ||
          (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVADDE) == SPR_AECR_OVADDE))
          )
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVADDE;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_CYADDE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

  temp4 = temp1;
  if (temp4 == temp1)
    or1k_mstats.byteadd++;
}
INSTRUCTION (l_addc) {
  orreg_t temp1, temp2, temp3;
  int8_t temp4;
  int    carry_in = (cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY;

  temp2 = (orreg_t)PARAM2;
  temp3 = (orreg_t)PARAM1;
  temp1 = temp2 + temp3;

  if(carry_in)
    {
      temp1++;				/* Add in the carry bit */
    }

  SET_PARAM0(temp1);

  /* Set overflow if two negative values gave a positive sum, or if two
     positive values gave a negative sum. Otherwise clear it. There are no
     corner cases with the extra bit carried in (unlike the carry flag - see
     below). */
  if ((((long int) temp2 <  0) && 
       ((long int) temp3 <  0) &&
       ((long int) temp1 >= 0)) ||
      (((long int) temp2 >= 0) && 
       ((long int) temp3 >= 0) &&
       ((long int) temp1 <  0)))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* Set the carry flag if (as unsigned values) the result is smaller than
     either operand (if it smaller than one, it will be smaller than both, so
     we need only test one). If there is a carry in, the test should be less
     than or equal, to deal with the 0 + 0xffffffff + c = 0 case (which
     generates a carry). */
  if ((carry_in && ((uorreg_t) temp1 <= (uorreg_t) temp2)) ||
      ((uorreg_t) temp1 < (uorreg_t) temp2))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_CY;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_CY;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for either CY or OV, and set
         appropriate AESR bit, then raise exception */
      if ((((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_CYADDE) == SPR_AECR_CYADDE)) ||
          (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVADDE) == SPR_AECR_OVADDE))
          )
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVADDE;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_CYADDE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

  temp4 = temp1;
  if (temp4 == temp1)
    or1k_mstats.byteadd++;
}
INSTRUCTION (l_swa) {
  int old_cyc = 0;
  uint32_t swa_success = 0;

  if (config.cpu.sbuf_len)
    old_cyc = runtime.sim.mem_cycles;

  if (cpu_state.loadlock_addr == dmmu_translate (PARAM0, 0)
      && cpu_state.loadlock_active) {
    swa_success = 1;
    set_mem32(PARAM0, PARAM1, &breakpoint);
    if (config.cpu.sbuf_len) {
      int t = runtime.sim.mem_cycles;
      runtime.sim.mem_cycles = old_cyc;
      sbuf_store (t - old_cyc);
    }
  }

  if (swa_success)
      cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;

  cpu_state.loadlock_active = 0;
}
INSTRUCTION (l_sw) {
  int old_cyc = 0;
  if (config.cpu.sbuf_len) old_cyc = runtime.sim.mem_cycles;
  set_mem32(PARAM0, PARAM1, &breakpoint);
  if (config.cpu.sbuf_len) {
    int t = runtime.sim.mem_cycles;
    runtime.sim.mem_cycles = old_cyc;
    sbuf_store (t - old_cyc);
  }
}
INSTRUCTION (l_sb) {
  int old_cyc = 0;
  if (config.cpu.sbuf_len) old_cyc = runtime.sim.mem_cycles;
  set_mem8(PARAM0, PARAM1, &breakpoint);
  if (config.cpu.sbuf_len) {
    int t = runtime.sim.mem_cycles;
    runtime.sim.mem_cycles = old_cyc;
    sbuf_store (t- old_cyc);
  }
}
INSTRUCTION (l_sh) {
  int old_cyc = 0;
  if (config.cpu.sbuf_len) old_cyc = runtime.sim.mem_cycles;
  set_mem16(PARAM0, PARAM1, &breakpoint);
  if (config.cpu.sbuf_len) {
    int t = runtime.sim.mem_cycles;
    runtime.sim.mem_cycles = old_cyc;
    sbuf_store (t - old_cyc);
  }
}
INSTRUCTION (l_lwa) {
  uint32_t val;
  if (config.cpu.sbuf_len)
    sbuf_load ();
  val = eval_mem32(PARAM1, &breakpoint);
  cpu_state.loadlock_active = 1;
  cpu_state.loadlock_addr = dmmu_translate (PARAM1, 0);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lws) {
  uint32_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem32(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lwz) {
  uint32_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem32(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lbs) {
  int8_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem8(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lbz) {  
  uint8_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem8(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lhs) {  
  int16_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem16(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_lhz) {  
  uint16_t val;
  if (config.cpu.sbuf_len) sbuf_load ();
  val = eval_mem16(PARAM1, &breakpoint);
  /* If eval operand produced exception don't set anything. JPB changed to
     trigger on breakpoint, as well as except_pending (seemed to be a bug). */
  if (!(except_pending || breakpoint))
    SET_PARAM0(val);
}
INSTRUCTION (l_movhi) {
  SET_PARAM0(PARAM1 << 16);
}
INSTRUCTION (l_and) {
  uorreg_t temp1;
  temp1 = PARAM1 & PARAM2;
  SET_PARAM0(temp1);
}
INSTRUCTION (l_or) {
  uorreg_t temp1;
  temp1 = PARAM1 | PARAM2;
  SET_PARAM0(temp1);
}
INSTRUCTION (l_xor) {
  /* The argument is now specified as unsigned, but historically OR1K has
     always treated the argument as signed (so l.xori rD,rA,-1 can be used in
     the absence of l.not). Use this as the default behavior. This is
     controlled from or32.c. */
  uorreg_t  temp1 = PARAM1 ^ PARAM2;
  SET_PARAM0(temp1);
}
INSTRUCTION (l_sub) {
  orreg_t temp1, temp2, temp3;

  temp3 = (orreg_t)PARAM2;
  temp2 = (orreg_t)PARAM1;
  temp1 = temp2 - temp3;
  SET_PARAM0 (temp1);

  /* Set overflow if a negative value minus a positive value gave a positive
     sum, or if a positive value minus a negative value gave a negative
     sum. Otherwise clear it */
  if ((((long int) temp2 <  0) && 
       ((long int) temp3 >= 0) &&
       ((long int) temp1 >= 0)) ||
      (((long int) temp2 >= 0) && 
       ((long int) temp3 <  0) &&
       ((long int) temp1 <  0)))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* Set the carry flag if (as unsigned values) the second operand is greater
     than the first. */
  if ((uorreg_t) temp3 > (uorreg_t) temp2)
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_CY;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_CY;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for either CY or OV, and set
         appropriate AESR bit, then raise exception */
      if ((((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_CYADDE) == SPR_AECR_CYADDE)) ||
          (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVADDE) == SPR_AECR_OVADDE))
          )
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVADDE;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_CYADDE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }
}
/*int mcount = 0;*/
INSTRUCTION (l_mul) {
  orreg_t   temp0, temp1, temp2;
  LONGEST   ltemp0, ltemp1, ltemp2;

  /* Args in 32-bit */
  temp2 = (orreg_t) PARAM2;
  temp1 = (orreg_t) PARAM1;

  /* Compute initially in 64-bit */
  ltemp1 = (LONGEST) temp1;
  ltemp2 = (LONGEST) temp2;
/** MoMA begin **/
/* backup begin */
  ltemp0 = ltemp1 * ltemp2;
/* backup end 
  ltemp0 = (ltemp1 * ltemp2) % 1000;
/** MoMA end **/
  temp0  = (orreg_t) (ltemp0  & 0xffffffffLL);
  SET_PARAM0 (temp0);

  /* We have 2's complement overflow, if the result is less than the smallest
     possible 32-bit negative number, or greater than the largest possible
     32-bit positive number. */
  if ((ltemp0 < (LONGEST) INT32_MIN) || (ltemp0 > (LONGEST) INT32_MAX))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* CY flag is not touched */

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for OV, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVMULE) == SPR_AECR_OVMULE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVMULE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

}
INSTRUCTION (l_mulu) {
  uorreg_t   temp0, temp1, temp2;
  ULONGEST  ultemp0, ultemp1, ultemp2;

  /* Args in 32-bit */
  temp2 = (uorreg_t) PARAM2;
  temp1 = (uorreg_t) PARAM1;

  /* Compute initially in 64-bit */
  ultemp1 = (ULONGEST) temp1 & 0xffffffffULL;
  ultemp2 = (ULONGEST) temp2 & 0xffffffffULL;
  ultemp0 = ultemp1 * ultemp2;

  temp0  = (uorreg_t) (ultemp0  & 0xffffffffULL);
  SET_PARAM0 (temp0);

  /* OV flag is not touched */

  /* We have 1's complement overflow, if the result is greater than the
     largest possible 32-bit unsigned number. */
  if (ultemp0 > (ULONGEST) UINT32_MAX)
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_CY;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_CY;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for CY, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_CY) == SPR_SR_CY) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_CYMULE) == SPR_AECR_CYMULE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_CYMULE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    }

    /* mulu does not raise an exception when AECR/AESR isn't in use */
    
  }

}
INSTRUCTION (l_div) {
  orreg_t  temp3, temp2, temp1;
  
  temp3 = (orreg_t) PARAM2;
  temp2 = (orreg_t) PARAM1;
 
  /* Check for divide by zero (sets carry) */
  /* INT32_MIN / -1 should will also overflow */
  if ((0 == temp3) ||
      (0x80000000 == temp2 && 0xffffffff == temp3))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      temp1 = temp2 / temp3;
      SET_PARAM0(temp1);
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for either CY or OV, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_DBZE) == SPR_AECR_DBZE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_DBZE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

}
INSTRUCTION (l_divu) {
  uorreg_t temp3, temp2, temp1;
  
  temp3 = (uorreg_t) PARAM2;
  temp2 = (uorreg_t) PARAM1;
 
 /* Check for divide by zero (sets carry) */
  if (0 == temp3)
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      temp1 = temp2 / temp3;
      SET_PARAM0(temp1);
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }

  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
        
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for either CY or OV, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_DBZE) == SPR_AECR_DBZE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_DBZE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

}
INSTRUCTION (l_sll) {
  uorreg_t temp1;

  temp1 = PARAM1 << PARAM2;
  SET_PARAM0(temp1);
  /* runtime.sim.cycles += 2; */
}
INSTRUCTION (l_sra) {
  orreg_t temp1;
  
/** MoMA begin **/
/* backup begin */
  temp1 = (orreg_t)PARAM1 >> PARAM2;
/* backup end 
  temp1 = (orreg_t)(PARAM1 >> PARAM2) % 1000;
/** MoMA end **/
  SET_PARAM0(temp1);
  /* runtime.sim.cycles += 2; */
}
INSTRUCTION (l_srl) {
  uorreg_t temp1;
  temp1 = PARAM1 >> PARAM2;
  SET_PARAM0(temp1);
  /* runtime.sim.cycles += 2; */
}
INSTRUCTION (l_ror) {
  uorreg_t temp1;
  temp1  = PARAM1 >> (PARAM2 & 0x1f);
  temp1 |= PARAM1 << (32 - (PARAM2 & 0x1f));
  SET_PARAM0(temp1);
}
INSTRUCTION (l_bf) {
  if (config.bpb.enabled) {
    int fwd = (PARAM0 >= cpu_state.pc) ? 1 : 0;
    or1k_mstats.bf[cpu_state.sprs[SPR_SR] & SPR_SR_F ? 1 : 0][fwd]++;
    bpb_update(current->insn_addr, cpu_state.sprs[SPR_SR] & SPR_SR_F ? 1 : 0);
  }
  if(cpu_state.sprs[SPR_SR] & SPR_SR_F) {
    if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
      pcnext = cpu_state.pc + (orreg_t)PARAM0 * 4;
    } else {
      cpu_state.pc_delay = cpu_state.pc + (orreg_t)PARAM0 * 4;
      next_delay_insn = 1;
    }
    btic_update(pcnext);
  } else {
    btic_update(cpu_state.pc);
  }
}
INSTRUCTION (l_bnf) {
  if (config.bpb.enabled) {
    int fwd = (PARAM0 >= cpu_state.pc) ? 1 : 0;
    or1k_mstats.bnf[cpu_state.sprs[SPR_SR] & SPR_SR_F ? 0 : 1][fwd]++;
    bpb_update(current->insn_addr, cpu_state.sprs[SPR_SR] & SPR_SR_F ? 0 : 1);
  }
  if (!(cpu_state.sprs[SPR_SR] & SPR_SR_F)) {
    if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
      pcnext = cpu_state.pc + (orreg_t)PARAM0 * 4;
    } else {
      cpu_state.pc_delay = cpu_state.pc + (orreg_t)PARAM0 * 4;
      next_delay_insn = 1;
    }
    btic_update(pcnext);
  } else {
    btic_update(cpu_state.pc);
  }
}
INSTRUCTION (l_j) {
  if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
    pcnext = cpu_state.pc + (orreg_t)PARAM0 * 4;
  } else {
    cpu_state.pc_delay = cpu_state.pc + (orreg_t)PARAM0 * 4;
    next_delay_insn = 1;
  }
}
INSTRUCTION (l_jal) {
  if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
    pcnext = cpu_state.pc + (orreg_t)PARAM0 * 4;

    setsim_reg(LINK_REGNO, cpu_state.pc + 4);

    if (config.sim.profile) {
      struct label_entry *tmp;
      if (verify_memoryarea(pcnext) && (tmp = get_label (pcnext)))
        fprintf (runtime.sim.fprof, "+%08llX %"PRIxADDR" %"PRIxADDR" %s\n",
                 runtime.sim.cycles, cpu_state.pc + 4, pcnext,
                 tmp->name);
      else
        fprintf (runtime.sim.fprof, "+%08llX %"PRIxADDR" %"PRIxADDR" @%"PRIxADDR"\n",
                 runtime.sim.cycles, cpu_state.pc + 4, pcnext,
                 pcnext);
    }
  } else {
    cpu_state.pc_delay = cpu_state.pc + (orreg_t)PARAM0 * 4;
  
    setsim_reg(LINK_REGNO, cpu_state.pc + 8);
    next_delay_insn = 1;
    if (config.sim.profile) {
      struct label_entry *tmp;
      if (verify_memoryarea(cpu_state.pc_delay) && (tmp = get_label (cpu_state.pc_delay)))
        fprintf (runtime.sim.fprof, "+%08llX %"PRIxADDR" %"PRIxADDR" %s\n",
                 runtime.sim.cycles, cpu_state.pc + 8, cpu_state.pc_delay,
                 tmp->name);
      else
        fprintf (runtime.sim.fprof, "+%08llX %"PRIxADDR" %"PRIxADDR" @%"PRIxADDR"\n",
                 runtime.sim.cycles, cpu_state.pc + 8, cpu_state.pc_delay,
                 cpu_state.pc_delay);
    }
  }
}
INSTRUCTION (l_jalr) {
  /* Badly aligned destination or use of link register triggers an exception */
  uorreg_t  temp1 = PARAM0;

  if (REG_PARAM0 == LINK_REGNO)
    {
      except_handle (EXCEPT_ILLEGAL, cpu_state.pc);
    }
  else if ((temp1 & 0x3) != 0)
    {
      except_handle (EXCEPT_ALIGN, cpu_state.pc);
    }
  else
    {
      if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
        pcnext = temp1;
        setsim_reg(LINK_REGNO, cpu_state.pc + 4);
      } else {
        cpu_state.pc_delay = temp1;
        setsim_reg(LINK_REGNO, cpu_state.pc + 8);
        next_delay_insn = 1;
      }
    }
}
INSTRUCTION (l_jr) {
  /* Badly aligned destination triggers an exception */
  uorreg_t  temp1 = PARAM0;

  if ((temp1 & 0x3) != 0)
    {
      except_handle (EXCEPT_ALIGN, cpu_state.pc);
    }
  else
    {
      if(cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_ND) {
        pcnext = temp1;

        if (config.sim.profile)
          {
            fprintf (runtime.sim.fprof, "-%08llX %"PRIxADDR"\n",
                     runtime.sim.cycles, pcnext);
          }
      } else {
        cpu_state.pc_delay = temp1;
        next_delay_insn = 1;

        if (config.sim.profile)
          {
            fprintf (runtime.sim.fprof, "-%08llX %"PRIxADDR"\n",
                     runtime.sim.cycles, cpu_state.pc_delay);
          }
      }
    }
}
INSTRUCTION (l_rfe) {
  pcnext = cpu_state.sprs[SPR_EPCR_BASE];
  mtspr(SPR_SR, cpu_state.sprs[SPR_ESR_BASE]);
}
INSTRUCTION (l_nop) {
  uint32_t k = PARAM0;
  switch (k)
    {
    case NOP_NOP:
      break;
    case NOP_EXIT:
      PRINTFQ("exit(%"PRIdREG")\n", evalsim_reg (3));
      PRINTFQ("@reset : cycles %lld, insn #%lld\n",
              runtime.sim.reset_cycles, runtime.cpu.reset_instructions);
      PRINTFQ("@exit  : cycles %lld, insn #%lld\n", runtime.sim.cycles,
              runtime.cpu.instructions);
      PRINTFQ(" diff  : cycles %lld, insn #%lld\n",
              runtime.sim.cycles - runtime.sim.reset_cycles,
              runtime.cpu.instructions - runtime.cpu.reset_instructions);
    case NOP_EXIT_SILENT:
      if (config.sim.is_library)
	{
	  runtime.cpu.halted = 1;
	  set_stall_state (1);
	}
      else
	{
	  sim_done();
	}
      break;
    case NOP_REPORT:
      PRINTF("report(0x%"PRIxREG");\n", evalsim_reg(3));
      break;
    case NOP_PUTC:		/*JPB */
      printf( "%c", (char)(evalsim_reg( 3 ) & 0xff));
      fflush( stdout );
      break;
    case NOP_CNT_RESET:
      PRINTF("****************** counters reset ******************\n");
      PRINTF("cycles %lld, insn #%lld\n", runtime.sim.cycles, runtime.cpu.instructions); 
      PRINTF("****************** counters reset ******************\n");
      runtime.sim.reset_cycles = runtime.sim.cycles;
      runtime.cpu.reset_instructions = runtime.cpu.instructions;
      break;    
    case NOP_GET_TICKS:
      cpu_state.reg[11] = runtime.sim.cycles & 0xffffffff;
      cpu_state.reg[12] = runtime.sim.cycles >> 32;
      break;
    case NOP_GET_PS:
      cpu_state.reg[11] = config.sim.clkcycle_ps;
      break;
    case NOP_TRACE_ON:
      runtime.sim.hush = 0;
      break;
    case NOP_TRACE_OFF:
      runtime.sim.hush = 1;
      break;
    case NOP_RANDOM:
      cpu_state.reg[11] = (unsigned int) (random () & 0xffffffff);
      break;
    case NOP_OR1KSIM:
      cpu_state.reg[11] = 1;
      break;
    default:
      break;
  }
}
INSTRUCTION (l_sfeq) {
  if(PARAM0 == PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfne) {
  if(PARAM0 != PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfgts) {
  if((orreg_t)PARAM0 > (orreg_t)PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfges) {
  if((orreg_t)PARAM0 >= (orreg_t)PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sflts) {
  if((orreg_t)PARAM0 < (orreg_t)PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfles) {
  if((orreg_t)PARAM0 <= (orreg_t)PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfgtu) {
  if(PARAM0 > PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfgeu) {
  if(PARAM0 >= PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfltu) {
  if(PARAM0 < PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_sfleu) {
  if(PARAM0 <= PARAM1)
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}
INSTRUCTION (l_extbs) {
  int8_t x;
  x = PARAM1;
  SET_PARAM0((orreg_t)x);
}
INSTRUCTION (l_extbz) {
  uint8_t x;
  x = PARAM1;
  SET_PARAM0((uorreg_t)x);
}
INSTRUCTION (l_exths) {
  int16_t x;
  x = PARAM1;
  SET_PARAM0((orreg_t)x);
}
INSTRUCTION (l_exthz) {
  uint16_t x;
  x = PARAM1;
  SET_PARAM0((uorreg_t)x);
}
INSTRUCTION (l_extws) {
  int32_t x;
  x = PARAM1;
  SET_PARAM0((orreg_t)x);
}
INSTRUCTION (l_extwz) {
  uint32_t x;
  x = PARAM1;
  SET_PARAM0((uorreg_t)x);
}
INSTRUCTION (l_mtspr) {
  uint16_t regno = PARAM0 | PARAM2;
  uorreg_t value = PARAM1;

  if (cpu_state.sprs[SPR_SR] & SPR_SR_SM)
    mtspr(regno, value);
  else {
    PRINTF("WARNING: trying to write SPR while SR[SUPV] is cleared.\n");
    sim_done();
  }
}
INSTRUCTION (l_mfspr) {
  uint16_t regno = PARAM1 | PARAM2;
  uorreg_t value = mfspr(regno);

  if ((cpu_state.sprs[SPR_SR] & SPR_SR_SM) ||
      // TODO: Check if this SPR should actually be allowed to be read with
      // SR's SM==0 and SUMRA==1
      (!(cpu_state.sprs[SPR_SR] & SPR_SR_SM) && 
       (cpu_state.sprs[SPR_SR] & SPR_SR_SUMRA)))
    SET_PARAM0(value);
  else
    {
      SET_PARAM0(0);
      PRINTF("WARNING: trying to read SPR while SR[SUPV] and SR[SUMRA] is cleared.\n");
      sim_done();
  }
}
INSTRUCTION (l_sys) {
  except_handle(EXCEPT_SYSCALL, cpu_state.sprs[SPR_EEAR_BASE]);
}
INSTRUCTION (l_trap) {
  /* TODO: some SR related code here! */
  except_handle(EXCEPT_TRAP, cpu_state.sprs[SPR_EEAR_BASE]);
}
INSTRUCTION (l_mac) {
  orreg_t lo, hi;
  LONGEST acc, prod, tmp;
  LONGEST x, y;

  lo = (orreg_t) cpu_state.sprs[SPR_MACLO];
  hi = (orreg_t) cpu_state.sprs[SPR_MACHI];
  x = (orreg_t) PARAM0;
  y = (orreg_t) PARAM1;

  /* current accumulator value as 64-bit signed int */
  acc = ((LONGEST)lo & 0xFFFFFFFFULL) | ((LONGEST)hi << 32);
  /* compute 32x32-bit to 64-bit product */
  prod = x * y;
  /* 64-bit addition */
  tmp = acc + prod;
  
  /* This implementation is very fast - it needs only one cycle for mac.  */
  lo = tmp & 0xFFFFFFFFULL;
  hi = (tmp >> 32) & 0xFFFFFFFFULL;
  cpu_state.sprs[SPR_MACLO] = lo;
  cpu_state.sprs[SPR_MACHI] = hi;

  /* Set overflow if two negative values gave a positive sum, or if two
     positive values gave a negative sum. Otherwise clear it */
  if (((acc <  0) && 
       (prod <  0) &&
       (tmp >= 0)) ||
      ((acc >= 0) && 
       (prod >= 0) &&
       (tmp <  0)))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }
  
  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
    
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for OV, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVMACADDE) == SPR_AECR_OVMACADDE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVMACADDE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

}
INSTRUCTION (l_msb) {
  orreg_t lo, hi;
  LONGEST acc, prod, tmp;
  LONGEST x, y;

  lo = (orreg_t) cpu_state.sprs[SPR_MACLO];
  hi = (orreg_t) cpu_state.sprs[SPR_MACHI];
  x = (orreg_t) PARAM0;
  y = (orreg_t) PARAM1;

  /* current accumulator value as 64-bit signed int */
  acc = ((LONGEST)lo & 0xFFFFFFFFULL) | ((LONGEST)hi << 32);
  /* compute 32x32-bit to 64-bit product */
  prod = x * y;
  /* 64-bit subtraction */
  tmp = acc - prod;
  
  /* This implementation is very fast - it needs only one cycle for mac.  */
  lo = tmp & 0xFFFFFFFFULL;
  hi = (tmp >> 32) & 0xFFFFFFFFULL;
  cpu_state.sprs[SPR_MACLO] = lo;
  cpu_state.sprs[SPR_MACHI] = hi;

  /* Set overflow if a negative value minus a positive value gave a positive
     sum, or if a positive value minus a negative value gave a negative
     sum. Otherwise clear it */
  if (((acc <  0) && 
       (prod >= 0) &&
       (tmp >= 0)) ||
      ((acc >= 0) && 
       (prod <  0) &&
       (tmp <  0)))
    {
      cpu_state.sprs[SPR_SR] |= SPR_SR_OV;
    }
  else
    {
      cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV;
    }
  
  /* Trigger a range exception if the overflow flag is set and the SR[OVE] bit
     is set. */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) == SPR_SR_OVE) {
    
    /* Check if using precise arithmetic exceptions */
    if ((cpu_state.sprs[SPR_CPUCFGR] & SPR_CPUCFGR_AECSRP) == SPR_CPUCFGR_AECSRP) {
    
      /* If using AECR/AESR, check for OV, and set
         appropriate AESR bit, then raise exception */
      if (((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV) && ((cpu_state.sprs[SPR_AECR] & SPR_AECR_OVMACADDE) == SPR_AECR_OVMACADDE))
        {
          cpu_state.sprs[SPR_AESR] = 0;
          if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV) == SPR_SR_OV)
            cpu_state.sprs[SPR_AESR] |= SPR_AESR_OVMACADDE;
          except_handle (EXCEPT_RANGE, cpu_state.pc);
        }

    } else {
      
      /* No AECR/AESR, only raise exception if OV is set */
      if ((cpu_state.sprs[SPR_SR] & SPR_SR_OV)  == SPR_SR_OV)
        except_handle (EXCEPT_RANGE, cpu_state.pc);

    }
    
  }

}
INSTRUCTION (l_macrc) {
  orreg_t lo;
  /* No need for synchronization here -- all MAC instructions are 1 cycle long.  */
  lo =  cpu_state.sprs[SPR_MACLO];
  //PRINTF ("<%08x>\n", (unsigned long)l);
  SET_PARAM0(lo);
  cpu_state.sprs[SPR_MACLO] = 0;
  cpu_state.sprs[SPR_MACHI] = 0;
}
INSTRUCTION (l_cmov) {
  SET_PARAM0(cpu_state.sprs[SPR_SR] & SPR_SR_F ? PARAM1 : PARAM2);
}
INSTRUCTION (l_ff1) {
  SET_PARAM0(ffs(PARAM1));
}
INSTRUCTION (l_fl1) {
  orreg_t t = (orreg_t)PARAM1;

  /* Reverse the word and use ffs */
  t = (((t & 0xaaaaaaaa) >> 1) | ((t & 0x55555555) << 1));
  t = (((t & 0xcccccccc) >> 2) | ((t & 0x33333333) << 2));
  t = (((t & 0xf0f0f0f0) >> 4) | ((t & 0x0f0f0f0f) << 4));
  t = (((t & 0xff00ff00) >> 8) | ((t & 0x00ff00ff) << 8));
  t = ffs ((t >> 16) | (t << 16));
  
  SET_PARAM0 (0 == t ? t : 33 - t);
}
/******* Floating point instructions *******/
/* Do calculation, and update FPCSR as required */
/* Single precision */
INSTRUCTION (lf_add_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_add((unsigned int)PARAM1,(unsigned int)PARAM2));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_div_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_div((unsigned int)PARAM1,(unsigned int)PARAM2));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_ftoi_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_to_int32((unsigned int)PARAM1));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_itof_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(int32_to_float32((unsigned int)PARAM1));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_madd_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_add((unsigned int)PARAM0, float32_mul((unsigned int)PARAM1,(unsigned int)PARAM2)));
  // Note: this ignores flags from the multiply!
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_mul_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_mul((unsigned int)PARAM1,(unsigned int)PARAM2));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_rem_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_rem((unsigned int)PARAM1,(unsigned int)PARAM2));
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sfeq_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if(float32_eq((unsigned int)PARAM0, (unsigned int)PARAM1))
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sfge_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if((!float32_lt((unsigned int)PARAM0, (unsigned int)PARAM1) & 
      !float32_is_nan( (unsigned int)PARAM0) &
      !float32_is_nan( (unsigned int)PARAM1) ) )
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;  
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sfgt_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if((!float32_le((unsigned int)PARAM0, (unsigned int)PARAM1)  & 
      !float32_is_nan( (unsigned int)PARAM0) &
      !float32_is_nan( (unsigned int)PARAM1) ) )
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sfle_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if((float32_le((unsigned int)PARAM0, (unsigned int)PARAM1) & 
      !float32_is_nan( (unsigned int)PARAM0) &
      !float32_is_nan( (unsigned int)PARAM1) ) )    
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sflt_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if(( float32_lt((unsigned int)PARAM0, (unsigned int)PARAM1) & 
       !float32_is_nan( (unsigned int)PARAM0) &
       !float32_is_nan( (unsigned int)PARAM1) ) )
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sfne_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  if(!float32_eq((unsigned int)PARAM0, (unsigned int)PARAM1))    
    cpu_state.sprs[SPR_SR] |= SPR_SR_F;
  else
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
  float_set_flags();
  } else l_invalid();
}
INSTRUCTION (lf_sub_s) {
  if (config.cpu.hardfloat) {
  float_set_rm();
  SET_PARAM0(float32_sub((unsigned int)PARAM1,(unsigned int)PARAM2));
  float_set_flags();
  } else l_invalid();
}

/******* Custom instructions *******/
INSTRUCTION (l_cust1) {
  /*int destr = current->insn >> 21;
    int src1r = current->insn >> 15;
    int src2r = current->insn >> 9;*/
}
INSTRUCTION (l_cust2) {
}
INSTRUCTION (l_cust3) {
}
INSTRUCTION (l_cust4) {
}
INSTRUCTION (lf_cust1) {
}

/** MoMA begin **/
/* MOD instruction */
INSTRUCTION (l_mod) {
  orreg_t temp1;
  temp1 = (orreg_t)PARAM1 % PARAM2;
  SET_PARAM0(temp1);
}

INSTRUCTION (le3_modmul2048) {
	unsigned ins_reglen = 2048;
	//printf("modmul\n");
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;
	orreg_t mC = PARAM3;
	
	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_mC;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init(mpz_mC);
	
	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetB = MOMA_BASEADDR + ((unsigned)(mB) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetC = MOMA_BASEADDR + ((unsigned)(mC) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));

	int reglen_bytes = ins_reglen / MOMA_STDWORDSIZE;

	mpz_t baseWord;
	mpz_init_set_str(baseWord, MOMA_STDHEXBASE, 16);
	
	for (int i  = 0; i < reglen_bytes; i++)
	{
		mpz_mul(mpz_mA, mpz_mA, baseWord);
		mpz_add_ui(mpz_mA, mpz_mA, cpu_state.sprs[offsetA+i]);

		mpz_mul(mpz_mB, mpz_mB, baseWord);
		mpz_add_ui(mpz_mB, mpz_mB, cpu_state.sprs[offsetB+i]);

		mpz_mul(mpz_mC, mpz_mC, baseWord);
		mpz_add_ui(mpz_mC, mpz_mC, cpu_state.sprs[offsetC+i]);
	}

	mpz_mul(mpz_mD, mpz_mA, mpz_mB);
	mpz_mod(mpz_mD, mpz_mD, mpz_mC);

	//gmp_printf("> %Zd * %Zd mod %Zd = %Zd\n", mpz_mA, mpz_mB, mpz_mC, mpz_mD);

	//gmp_printf("%Zx * %Zx mod %Zx = %Zx\n", mpz_mA, mpz_mB, mpz_mC, mpz_mD);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mC);
	
	for (int i = reglen_bytes - 1; i >= 0; i--)
	{
		cpu_state.sprs[offsetD+i] = (uorreg_t) mpz_get_ui(mpz_mD);
		mpz_tdiv_q(mpz_mD, mpz_mD, baseWord);
	}
	mpz_clear(mpz_mD);
}

INSTRUCTION (le3_modexp2048) {
	unsigned ins_reglen = 2048;
	//printf("modexp\n");
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;
	orreg_t mC = PARAM3;
	
	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_mC;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init(mpz_mC);
	
	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetB = MOMA_BASEADDR + ((unsigned)(mB) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetC = MOMA_BASEADDR + ((unsigned)(mC) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));

	int reglen_bytes = ins_reglen / MOMA_STDWORDSIZE;

	mpz_t baseWord;
	mpz_init_set_str(baseWord, MOMA_STDHEXBASE, 16);
	
	for (int i  = 0; i < reglen_bytes; i++)
	{
		mpz_mul(mpz_mA, mpz_mA, baseWord);
		mpz_add_ui(mpz_mA, mpz_mA, cpu_state.sprs[offsetA+i]);

		mpz_mul(mpz_mB, mpz_mB, baseWord);
		mpz_add_ui(mpz_mB, mpz_mB, cpu_state.sprs[offsetB+i]);

		mpz_mul(mpz_mC, mpz_mC, baseWord);
		mpz_add_ui(mpz_mC, mpz_mC, cpu_state.sprs[offsetC+i]);
	}

	mpz_powm(mpz_mD, mpz_mA, mpz_mB, mpz_mC);

	//gmp_printf("%Zd ^ %Zd mod %Zd = %Zd\n", mpz_mA, mpz_mB, mpz_mC, mpz_mD);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mC);
	
	for (int i = reglen_bytes - 1; i >= 0; i--)
	{
		cpu_state.sprs[offsetD+i] = (uorreg_t) mpz_get_ui(mpz_mD);
		mpz_tdiv_q(mpz_mD, mpz_mD, baseWord);
	}
	mpz_clear(mpz_mD);
}

INSTRUCTION (le3_gfun2048) {
	unsigned ins_reglen = 2048;
	//printf("gfun\n");
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;
	orreg_t mC = PARAM3;
	
	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_mC;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init(mpz_mC);
	
	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetB = MOMA_BASEADDR + ((unsigned)(mB) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetC = MOMA_BASEADDR + ((unsigned)(mC) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));

	int reglen_bytes = ins_reglen / MOMA_STDWORDSIZE;

	mpz_t baseWord;
	mpz_init_set_str(baseWord, MOMA_STDHEXBASE, 16);
	
	for (int i  = 0; i < reglen_bytes; i++)
	{
		mpz_mul(mpz_mA, mpz_mA, baseWord);
		mpz_add_ui(mpz_mA, mpz_mA, cpu_state.sprs[offsetA+i]);

		mpz_mul(mpz_mB, mpz_mB, baseWord);
		mpz_add_ui(mpz_mB, mpz_mB, cpu_state.sprs[offsetB+i]);

		mpz_mul(mpz_mC, mpz_mC, baseWord);
		mpz_add_ui(mpz_mC, mpz_mC, cpu_state.sprs[offsetC+i]);
	}

	mpz_t fkf, n, n2, xp1, xp2, ox, encZero;
	mpz_init(fkf);
	mpz_init(n);
	mpz_init(n2);
	mpz_init(xp1);
	mpz_init(xp2);
	mpz_init(ox);
	mpz_init(encZero);

	mpz_set_str(fkf, "3480", 10);
	mpz_set_str(n, "143", 10);
	mpz_mul(n2, n, n);
	mpz_set_str(xp1, "144", 10);
	mpz_set_str(xp2, "18304", 10);
	mpz_set_str(encZero, "12825", 10);

	mpz_powm(ox, mpz_mA, fkf, n2);

	if ((mpz_cmp(ox, xp1) < 0) || (mpz_cmp(xp2, ox) < 0))
	{
		mpz_set(mpz_mD, encZero);
	}
	else
	{
		mpz_set(mpz_mD, mpz_mB);
	}
	mpz_clear(fkf);
	mpz_clear(n);
	mpz_clear(n2);
	mpz_clear(xp1);
	mpz_clear(xp2);
	mpz_clear(ox);
	mpz_clear(encZero);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	//mpz_clear(mpz_mC);
	
	for (int i = reglen_bytes - 1; i >= 0; i--)
	{
		cpu_state.sprs[offsetD+i] = (uorreg_t) mpz_get_ui(mpz_mD);
		mpz_tdiv_q(mpz_mD, mpz_mD, baseWord);
	}
	mpz_clear(mpz_mD);
}

INSTRUCTION (le3_gcd2048) {
	unsigned ins_reglen = 2048;
	//printf("gcd\n");
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;
	
	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	
	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetB = MOMA_BASEADDR + ((unsigned)(mB) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));

	int reglen_bytes = ins_reglen / MOMA_STDWORDSIZE;

	mpz_t baseWord;
	mpz_init_set_str(baseWord, MOMA_STDHEXBASE, 16);

	for (int i  = 0; i < reglen_bytes; i++)
	{
		mpz_mul(mpz_mA, mpz_mA, baseWord);
		mpz_add_ui(mpz_mA, mpz_mA, cpu_state.sprs[offsetA+i]);

		mpz_mul(mpz_mB, mpz_mB, baseWord);
		mpz_add_ui(mpz_mB, mpz_mB, cpu_state.sprs[offsetB+i]);	
	}

	mpz_gcd(mpz_mD, mpz_mA, mpz_mB);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	
	for (int i = reglen_bytes - 1; i >= 0; i--)
	{
		cpu_state.sprs[offsetD+i] = (uorreg_t) mpz_get_ui(mpz_mD);
		mpz_tdiv_q(mpz_mD, mpz_mD, baseWord);
	}
	mpz_clear(mpz_mD);
}

INSTRUCTION (le3_inv2048) {
	unsigned ins_reglen = 2048;
	//printf("inv\n");
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;
	
	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	
	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));
	int offsetB = MOMA_BASEADDR + ((unsigned)(mB) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN));

	int reglen_bytes = ins_reglen / MOMA_STDWORDSIZE;

	mpz_t baseWord;
	mpz_init_set_str(baseWord, MOMA_STDHEXBASE, 16);

	for (int i  = 0; i < reglen_bytes; i++)
	{
		mpz_mul(mpz_mA, mpz_mA, baseWord);
		mpz_add_ui(mpz_mA, mpz_mA, cpu_state.sprs[offsetA+i]);

		mpz_mul(mpz_mB, mpz_mB, baseWord);
		mpz_add_ui(mpz_mB, mpz_mB, cpu_state.sprs[offsetB+i]);	
	}

	mpz_invert(mpz_mD, mpz_mA, mpz_mB);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	
	for (int i = reglen_bytes - 1; i >= 0; i--)
	{
		cpu_state.sprs[offsetD+i] = (uorreg_t) mpz_get_ui(mpz_mD);
		mpz_tdiv_q(mpz_mD, mpz_mD, baseWord);
	}
	mpz_clear(mpz_mD);
}

INSTRUCTION (le3_mter2048) {
	unsigned ins_reglen = 2048;
	//printf("mter\n");
	orreg_t mD = PARAM0;
	orreg_t rA = PARAM1;
	orreg_t uimm = PARAM2;

	//printf("-> %u %u %u\n", mD, rA, uimm);

	int offsetD = MOMA_BASEADDR + ((unsigned)(mD) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN)) + (MOMA_NUMWORDS * MOMA_REGLEN / ins_reglen - 1 - (unsigned)(uimm));
	cpu_state.sprs[offsetD] = rA;
}

INSTRUCTION (le3_mfer2048) {
	unsigned ins_reglen = 2048;
	//printf("mfer\n");
	orreg_t rD;
	orreg_t mA = PARAM1;
	orreg_t uimm = PARAM2;

	int offsetA = MOMA_BASEADDR + ((unsigned)(mA) * (MOMA_NUMWORDS * ins_reglen / MOMA_REGLEN)) + (MOMA_NUMWORDS * MOMA_REGLEN / ins_reglen - 1 - (unsigned)(uimm));
	rD = cpu_state.sprs[offsetA];
	//printf("<- %u %u %u\n", rD, mA, uimm);

	SET_PARAM0(rD);
}

/* Instruction to get data from MoMA registers. l.mfspr could be used, but it may require supervisor mode *
INSTRUCTION (moma_get) {
	uorreg_t offset = PARAM1 | PARAM2;
	uorreg_t value;
	
	if ((offset >= 0) && (offset < MOMA_LENGTH))
		value = cpu_state.sprs[SPR_MOMA + offset];
	else
		value = cpu_state.sprs[SPR_MOMA_PARTLEN];
	
	SET_PARAM0(value);
}

/* Instruction to set the MoMA registers. l.mtspr could be used, but it requires supervisor mode */
/* moma.set rA,rB,K *
INSTRUCTION (moma_set) {
	uorreg_t offset = PARAM0;
	uorreg_t value = PARAM1 | PARAM2;
	
	int i;
	
	if ((offset >= 0) && (offset < MOMA_LENGTH))
		cpu_state.sprs[SPR_MOMA + offset] = value;
	else if (offset == MOMA_LENGTH + 1) {
		if ((value < MOMA_WORD_SIZE) || (value > MOMA_MAX_REG_SIZE)) value = MOMA_MAX_REG_SIZE;
		else {
			for (i = 0; value != 0; i++, value >> 1);
			value = 1 << (i-1);
		}
		cpu_state.sprs[SPR_MOMA_PARTLEN] = value;
	}
}

/* moma.seto rA, rB, K to moma.seto K(rA),rB,L *
INSTRUCTION (moma_seto) {
	uorreg_t k = PARAM2 & 0xFFC0;
	uorreg_t l = PARAM2 & 0x003F;

	uorreg_t offset = PARAM0 + k;
	uorreg_t value = PARAM1 | l;
	
	int i;
	
	if ((offset >= 0) && (offset < MOMA_LENGTH))
		cpu_state.sprs[SPR_MOMA + offset] = value;
	else if (offset == MOMA_LENGTH + 1) {
		if ((value < MOMA_WORD_SIZE) || (value > MOMA_MAX_REG_SIZE)) value = MOMA_MAX_REG_SIZE;
		else {
			for (i = 0; value != 0; i++, value >> 1);
			value = 1 << (i-1);
		}
		cpu_state.sprs[SPR_MOMA_PARTLEN] = value;
	}
}

/* moma.seti K,L *
INSTRUCTION (moma_seti) {
	uorreg_t offset = PARAM0;
	uorreg_t value = PARAM1;
		
	int i;
	
	if ((offset >= 0) && (offset < MOMA_LENGTH))
		cpu_state.sprs[SPR_MOMA + offset] = value;
	else if (offset == MOMA_LENGTH + 1) {
		if ((value < MOMA_WORD_SIZE) || (value > MOMA_MAX_REG_SIZE)) value = MOMA_MAX_REG_SIZE;
		else {
			for (i = 0; value != 0; i++, value >> 1);
			value = 1 << (i-1);
		}
		cpu_state.sprs[SPR_MOMA_PARTLEN] = value;
	}
}

/* XOR operation between SPR_MOMA_DATA_1 and SPR_MOMA_DATA_2. Result is stored in SPR_MOMA_DATA_R *
INSTRUCTION (moma_xor) {
	uorreg_t partlen = (uorreg_t) cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = (unsigned int) MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = (unsigned int) partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = (uorreg_t) PARAM0;
	uorreg_t l = (uorreg_t) PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_1 + offset] ^ cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
	}
}

/* dataR[K to L] = data1[K to L] and data2[K to L] *
INSTRUCTION (moma_and) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_1 + offset] & cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
	}
}

/* dataR[K to L] = data1[K to L] or data2[K to L] *
INSTRUCTION (moma_or) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_1 + offset] | cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
	}
}

/* if (N == 0) or (N ==  1) data1[K to L] = not data1[K to L] // in HW: if (N[1] == 0) */
/* if (N == 0) or (N ==  2) data2[K to L] = not data2[K to L] // in HW: if (N[0] == 0) */
/* if (N == 0) or (N == -1) dataR[K to L] = not dataR[K to L] // in HW: if (N[0] xor N[1] == 0) */
/*INSTRUCTION (moma_not) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	orreg_t n = PARAM2;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		if ((n & 0x1) == 0) cpu_state.sprs[SPR_MOMA_DATA_1 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_1 + offset];
		if ((n & 0x2) == 0) cpu_state.sprs[SPR_MOMA_DATA_2 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
		if (((n & 0x1) ^ (n & 0x2)) == 0) cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_R + offset];
	}
}*/
/*
INSTRUCTION (moma_not) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_1 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_1 + offset];
		cpu_state.sprs[SPR_MOMA_DATA_2 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
		cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_R + offset];
	}
}

INSTRUCTION (moma_not1) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_1 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_1 + offset];
	}
}

INSTRUCTION (moma_not2) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_2 + offset] = !cpu_state.sprs[SPR_MOMA_DATA_2 + offset];
	}
}

INSTRUCTION (moma_notr) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;
	unsigned int offset;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	for (; k <= l; k++) {
		offset = ratio * k;
		cpu_state.sprs[SPR_MOMA_DATA_R + offset] = cpu_state.sprs[SPR_MOMA_DATA_R + offset];
	}
}

/* HADD operation using strings and gmp library *
INSTRUCTION (moma_hadd) {
	uorreg_t partlen = cpu_state.sprs[SPR_MOMA_PARTLEN];
	unsigned int parts = MOMA_MAX_REG_SIZE / partlen;
	unsigned int ratio = partlen / MOMA_WORD_SIZE;

	uorreg_t k = PARAM0;
	uorreg_t l = PARAM1;
	if (l > parts) l = parts;

	unsigned int offset, i;

	// assign static variables to big numbers
	mpz_t n, data1, data2, data1x2, dataR, baseWord;
	mpz_init(n);
	mpz_init_set_str(baseWord, "100000000", 16);
	for (i = 0; i < ratio; i++) {
		mpz_mul(n, n, baseWord);
		mpz_add_ui(n, n, cpu_state.sprs[SPR_MOMA_N + i]);
	}

	
	// for each part in the range
	for (; k <= l; k++) {
		offset = ratio * k;

		// initializing big numbers
		mpz_init(data1);
		mpz_init(data2);
			
		// copy values of N, data_1 and data_2 to mpz_t variables
		for (i = 0; i < ratio; i++) {
			mpz_mul(data1, data1, baseWord);
			mpz_mul(data2, data2, baseWord);
			mpz_add_ui(data1, data1, cpu_state.sprs[SPR_MOMA_DATA_1 + offset + i]);
			mpz_add_ui(data2, data2, cpu_state.sprs[SPR_MOMA_DATA_2 + offset + i]);
		}
		
		// multiply
		mpz_init(data1x2);
		mpz_mul(data1x2, data1, data2);
		mpz_clear(data1);
		mpz_clear(data2);
		
		// modulo N
		mpz_init(dataR);
		mpz_mod(dataR, data1x2, n);
		mpz_clear(data1x2);
		
		// store result in data_r registers
		for (i = ratio-1; i >= 0; i--) {
			cpu_state.sprs[SPR_MOMA_DATA_R + offset + i] = (uorreg_t) mpz_get_ui(dataR);
			mpz_tdiv_q(dataR, dataR, baseWord);
		}
		mpz_clear(dataR);
	}
	mpz_clear(n);
	mpz_clear(baseWord);
}*/
/** MoMA end **/

