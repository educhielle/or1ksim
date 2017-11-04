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
  ltemp0 = ltemp1 * ltemp2;
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
  
  temp1 = (orreg_t)PARAM1 >> PARAM2;
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

// Secure Computation

INSTRUCTION (le_eadd) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();
	//printf("ees: %u\teds: %u\n", ees, eds);
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_add(mpz_mD, mpz_mA, mpz_mB);
	gmp_printf("le.eadd -> dD: %Zx\tdA: %Zx\tdB: %Zx\n", mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eand) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_and(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_enot) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	e3_mpz_not(mpz_mD, eds);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eor) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_ior(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_ecmov) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	if (mpz_cmp_ui(mpz_mD, 0) != 0)
	{
		e3_set_mpz(mpz_mD, mA, ees);
		e3_decrypt(mpz_mD, mpz_mD, eds); // remove when reencrypt
	}
	else
	{
		e3_set_mpz(mpz_mD, mB, ees);
		e3_decrypt(mpz_mD, mpz_mD, eds); // remove when reencrypt
	}

	e3_encrypt(mpz_mD, mpz_mD, eds); // replace by reencrypt
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
}

INSTRUCTION (le_edec) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	mpz_sub_ui(mpz_mD, mpz_mD, 1);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
}

INSTRUCTION (le_edivs) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();
	
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA;
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, ees);
		e3_decrypt(mpz_mB, mpz_mB, eds);
		unsigned signA = e3_get_sign_mpz(mpz_mA, eds);
		unsigned signB = e3_get_sign_mpz(mpz_mB, eds);

		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		mpz_tdiv_q(mpz_mD, mpz_mA, mpz_mB);
		if (signA ^ signB) e3_twos_complement(mpz_mD, eds);

		e3_encrypt(mpz_mD, mpz_mD, eds);
		e3_set_e3reg(mD, mpz_mD, ees);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, ees);
	}
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_edivu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA, mpz_mB;
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, ees);
		e3_decrypt(mpz_mA, mpz_mA, eds);

		mpz_tdiv_q(mpz_mD, mpz_mA, mpz_mB);

		e3_encrypt(mpz_mD, mpz_mD, eds);
		e3_set_e3reg(mD, mpz_mD, ees);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, ees);
	}
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_eeq) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) == 0);
	mpz_set_ui(mpz_mD, cmp);
	gmp_printf("le.eeq -> dD: %Zx\tdA: %Zx\tdB: %Zx\n", mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);
	
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_exor) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_xor(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_epows) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_limit;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mpz_limit, 1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	mpz_mul_2exp(mpz_limit, mpz_limit, eds);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	unsigned signA = e3_get_sign_mpz(mpz_mA, eds);

	if (signA) e3_twos_complement(mpz_mA, eds);
	mpz_powm(mpz_mD, mpz_mA, mpz_mB, mpz_limit);
	if (signA) e3_twos_complement(mpz_mD, eds);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
	mpz_clear(mpz_limit);
}

INSTRUCTION (le_epowu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_limit;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mpz_limit, 1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	mpz_mul_2exp(mpz_limit, mpz_limit, eds);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_powm(mpz_mD, mpz_mA, mpz_mB, mpz_limit);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
	mpz_clear(mpz_limit);
}

INSTRUCTION (le_eff1) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	
	mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);

	unsigned pos = e3_ff1_mpz(mpz_mA);
	mpz_set_ui(mpz_mD, pos);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mD);	
}

INSTRUCTION (le_efl1) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	
	mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);

	unsigned pos = e3_fl1_mpz(mpz_mA);
	mpz_set_ui(mpz_mD, pos);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mD);	
}

INSTRUCTION (le_eges) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	unsigned signA = e3_get_sign_mpz(mpz_mA, eds);
	unsigned signB = e3_get_sign_mpz(mpz_mB, eds);

	int signDiff = signB - signA;
	if (signDiff > 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff < 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{	
		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) <= 0) : (mpz_cmp(mpz_mA, mpz_mB) >= 0));
		mpz_set_ui(mpz_mD, cmp);
	}

	e3_set_e3reg(mD, mpz_mD, ees);
	e3_encrypt(mpz_mD, mpz_mD, eds);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_egeu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);

	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) >= 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_egts) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	unsigned signA = e3_get_sign_mpz(mpz_mA, eds);
	unsigned signB = e3_get_sign_mpz(mpz_mB, eds);

	int signDiff = signB - signA;
	if (signDiff > 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff < 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) < 0) : (mpz_cmp(mpz_mA, mpz_mB) > 0));
		mpz_set_ui(mpz_mD, cmp);
	}

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_egtu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) > 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_einc) {
	//printf("le.einc in\n");
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mD, mpz_mD, eds);
	gmp_printf("le.einc -> %Zx -> ", mpz_mD);
	mpz_add_ui(mpz_mD, mpz_mD, 1);
	gmp_printf("%Zx\n", mpz_mD);
	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
	//printf("le.einc out\n");
}

INSTRUCTION (le_eles) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	unsigned signA = e3_get_sign_mpz(mpz_mA, eds);
	unsigned signB = e3_get_sign_mpz(mpz_mB, eds);

	int signDiff = signB - signA;
	if (signDiff < 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff > 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) >= 0) : (mpz_cmp(mpz_mA, mpz_mB) <= 0));
		mpz_set_ui(mpz_mD, cmp);
	}

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eleu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) <= 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_elts) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	unsigned signA = e3_get_sign_mpz(mA, eds);
	unsigned signB = e3_get_sign_mpz(mB, eds);

	int signDiff = signB - signA;
	if (signDiff < 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff > 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) > 0) : (mpz_cmp(mpz_mA, mpz_mB) < 0));
		mpz_set_ui(mpz_mD, cmp);
	}

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eltu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) < 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eland) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp_ui(mpz_mA, 0) && mpz_cmp_ui(mpz_mB, 0));
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_elnot) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);

	unsigned cmp = !mpz_cmp_ui(mpz_mA, 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_elor) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp_ui(mpz_mA, 0) || mpz_cmp_ui(mpz_mB, 0));
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emods) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA;
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, ees);
		unsigned signA = e3_get_sign(mA, eds);
		unsigned signB = e3_get_sign(mB, eds);

		if (signA) e3_twos_complement(mpz_mA, eds);
		if (signB) e3_twos_complement(mpz_mB, eds);
		mpz_mod(mpz_mD, mpz_mA, mpz_mB);
		if (signA) e3_twos_complement(mpz_mD, eds);

		e3_encrypt(mpz_mD, mpz_mD, eds);
		e3_set_e3reg(mD, mpz_mD, ees);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, ees);
	}

	mpz_clear(mpz_mB);
}

INSTRUCTION (le_emodu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA;
		
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, ees);
		e3_decrypt(mpz_mA, mpz_mA, eds);

		mpz_mod(mpz_mD, mpz_mA, mpz_mB);

		e3_encrypt(mpz_mD, mpz_mD, eds);
		e3_set_e3reg(mD, mpz_mD, ees);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, ees);
	}
	
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_emacs) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, ees);
	unsigned signB = e3_get_sign(mB, ees);

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	if (signA) e3_twos_complement(mpz_mA, eds);
	if (signB) e3_twos_complement(mpz_mB, eds);
	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	if (signA ^ signB) e3_twos_complement(mpz_mA, eds);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emacu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emsbs) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	e3_decrypt(mpz_mD, mpz_mD, eds);
	unsigned signA = e3_get_sign_mpz(mA, eds);
	unsigned signB = e3_get_sign_mpz(mB, eds);

	if (signA) e3_twos_complement(mpz_mA, eds);
	if (signB) e3_twos_complement(mpz_mB, eds);
	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	if (!(signA ^ signB)) e3_twos_complement(mpz_mA, eds);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emsbu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_set_mpz(mpz_mD, mD, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	e3_decrypt(mpz_mD, mpz_mD, eds);

	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	e3_twos_complement(mpz_mA, ees);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emuls) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned signA = e3_get_sign_mpz(mpz_mA, eds);
	unsigned signB = e3_get_sign_mpz(mpz_mB, eds);
	if (signA) e3_twos_complement(mpz_mA, eds);
	if (signB) e3_twos_complement(mpz_mB, eds);
	mpz_mul(mpz_mD, mpz_mA, mpz_mB);
	if (signA ^ signB) e3_twos_complement(mpz_mD, eds);
	gmp_printf("le.emuls -> dD: %Zx\tdA: %Zx\tdB: %Zx\n", mpz_mD, mpz_mA, mpz_mB);
	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_emulu) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	mpz_mul(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_ene) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) != 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_erand) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	
	mpz_t mpz_mD;
	mpz_init(mpz_mD);

	e3_randomp2(mpz_mD, eds);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mD);
}

INSTRUCTION (le_eror) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, eds-1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	
	// rotating the bits lower than the shift to the MSB positions
	mpz_set_ui(mask, 1);
	mpz_mul_2exp(mask, mask, shift);
	mpz_sub_ui(mask, mask, 1);
	mpz_and(mpz_mB, mpz_mA, mask);
	mpz_mul_2exp(mpz_mB, mpz_mB, eds-shift);

	// shifting the higher bits right
	mpz_tdiv_q_2exp(mpz_mA, mpz_mA, shift);

	// merging parts
	mpz_xor(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_esll) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, eds-1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_mul_2exp(mpz_mD, mpz_mA, shift);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_esra) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, eds-1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);
	unsigned signA = e3_get_sign_mpz(mA, eds);

	if (signA) e3_twos_complement(mpz_mA, eds);
	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_tdiv_q_2exp(mpz_mD, mpz_mA, shift);
	e3_extend_sign_mpz(mD, signA, eds);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_esrl) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, eds-1);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_tdiv_q_2exp(mpz_mD, mpz_mA, shift);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_esub) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, ees);
	e3_set_mpz(mpz_mB, mB, ees);
	e3_decrypt(mpz_mA, mpz_mA, eds);
	e3_decrypt(mpz_mB, mpz_mB, eds);

	e3_twos_complement(mpz_mB, eds);
	mpz_add(mpz_mD, mpz_mA, mpz_mB);

	e3_encrypt(mpz_mD, mpz_mD, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

// Other

INSTRUCTION (le_enc) {
	unsigned ees = e3_get_effective_encrypted_size();
	unsigned eds = e3_get_effective_decrypted_size();
	
	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, eds);

	e3_encrypt(mpz_mD, mpz_mA, eds);
	e3_set_e3reg(mD, mpz_mD, ees);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_gbk) {
	e3_generate_boot_key();
}

INSTRUCTION (le_mafdtspr) {
	unsigned reglen_bit = e3_get_effective_encrypted_size();

	orreg_t esrD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	if (esrD == E3_PRI || esrD == E3_PUB || esrD == E3_MOD)
	{
		mpz_t mpz_esrD, mpz_mA, mpz_mB;
		mpz_init(mpz_esrD);
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);

		e3_decrypt_fused(mpz_esrD, mpz_mA, mpz_mB);
		e3_set_esr(esrD, mpz_esrD);

		mpz_clear(mpz_esrD);
		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}
}

INSTRUCTION (le_mabdtspr) {
	unsigned reglen_bit = e3_get_effective_encrypted_size();

	orreg_t esrD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	if (esrD == E3_PRI || esrD == E3_PUB || esrD == E3_MOD)
	{
		mpz_t mpz_esrD, mpz_mA, mpz_mB;
		mpz_init(mpz_esrD);
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);
	
		e3_decrypt_boot(mpz_esrD, mpz_mA, mpz_mB);

		e3_set_esr(esrD, mpz_esrD);

		mpz_clear(mpz_esrD);
		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}
}

INSTRUCTION (le_mabfdtspr) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

}

INSTRUCTION (le_maefspr) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

}

INSTRUCTION (le_mfftsk) {
	if (!e3_is_fused_key_generated) e3_generate_fused_key();

	e3_clear(cpu_state.e3esr[E3_SSTAT]);
	e3_copy(cpu_state.e3esr[E3_PUB], cpu_state.e3esr[E3_FPUB], E3_REGLEN);
	e3_copy(cpu_state.e3esr[E3_PRI], cpu_state.e3esr[E3_FPRI], E3_REGLEN);
	e3_copy(cpu_state.e3esr[E3_MOD], cpu_state.e3esr[E3_FMOD], E3_REGLEN);
	cpu_state.sprs[SPR_E3_CTRL0] = cpu_state.sprs[SPR_E3_CTRL0] | 0x1;
}

INSTRUCTION (le_mfbtsk) {
	if (!e3_is_boot_key_generated) e3_generate_boot_key();

	e3_clear(cpu_state.e3esr[E3_SSTAT]);
	e3_copy(cpu_state.e3esr[E3_PUB], cpu_state.e3esr[E3_BPUB], E3_REGLEN);
	e3_copy(cpu_state.e3esr[E3_PRI], cpu_state.e3esr[E3_BPRI], E3_REGLEN);
	e3_copy(cpu_state.e3esr[E3_MOD], cpu_state.e3esr[E3_BMOD], E3_REGLEN);
	cpu_state.sprs[SPR_E3_CTRL0] = cpu_state.sprs[SPR_E3_CTRL0] | 0x1;
}

INSTRUCTION (le_mfer) {
	//printf("le.mfer in\n");
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t rD;
	orreg_t mA = PARAM1;
	orreg_t uimm = PARAM2;
	uimm = e3_filter_imm(uimm);

	rD = cpu_state.e3reg[mA][uimm];

	SET_PARAM0(rD);
	//printf("le.mfer out\n");
}

INSTRUCTION (le_mter) {
	//printf("le.mter in\n");
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t rA = PARAM1;
	orreg_t uimm = PARAM2;

	uimm = e3_filter_imm(uimm);

	cpu_state.e3reg[mD][uimm] = rA;
	//printf("le.mter out\n");
}

INSTRUCTION (le_mfspr) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t esrA = PARAM1;

	if (esrA == E3_MOD || esrA == E3_PUB
	 || esrA == E3_FMOD || esrA == E3_FPUB
	 || esrA == E3_BMOD || esrA == E3_BPUB)
		e3_copy(cpu_state.e3reg[mD], cpu_state.e3esr[esrA], reglen_bit);
}

INSTRUCTION (le_mtspr) {
	unsigned reglen_bit = e3_get_effective_encrypted_size();

	orreg_t esrD = PARAM0;
	orreg_t mA = PARAM1;

	if (esrD == E3_MOD)
		e3_copy(cpu_state.e3esr[esrD],cpu_state.e3reg[mA], reglen_bit);
	else if (esrD == E3_PUB)
	{
		e3_clear(cpu_state.e3esr[E3_PRI]);
		e3_clear(cpu_state.e3esr[E3_SSTAT]);
		e3_copy(cpu_state.e3esr[esrD],cpu_state.e3reg[mA], reglen_bit);
	}
	else if (esrD == E3_PRI) // debug only
	{
		e3_copy(cpu_state.e3esr[esrD],cpu_state.e3reg[mA], reglen_bit);
	}
}

INSTRUCTION (le_sfbusy) {
	if (cpu_state.sprs[SPR_E3_STAT] & 0x1)
		cpu_state.sprs[SPR_SR] |= SPR_SR_F;
	else
		cpu_state.sprs[SPR_SR] &= ~SPR_SR_F;
}

INSTRUCTION (le_lw) {
	printf("lw in\n");
	uint32_t val;
	if (config.cpu.sbuf_len) sbuf_load ();
	
	orreg_t mD = PARAM0;
	orreg_t rA = PARAM1;
	orreg_t uimm = PARAM2;
	
	val = eval_mem32(rA, &breakpoint);
	//printf("mD: %u\trA: %x\tuimm: %u\tval: %u\texcept_pending: %u\n", mD, rA, uimm, val, except_pending);
	/* If eval operand produced exception don't set anything. JPB changed to
	trigger on breakpoint, as well as except_pending (seemed to be a bug). */
	if (!(except_pending || breakpoint))
		cpu_state.e3reg[mD][uimm] = val;
	printf("lw out\n");
}

INSTRUCTION (le_lw4096) {
	uint32_t val;
	if (config.cpu.sbuf_len) sbuf_load ();
	
	orreg_t mD = PARAM0;
	orreg_t rA = PARAM1;
	orreg_t uimm = PARAM2;
	rA += (E3_NUMWORDS-uimm-1) << 2;

	val = eval_mem32(rA, &breakpoint);
	/* If eval operand produced exception don't set anything. JPB changed to
	trigger on breakpoint, as well as except_pending (seemed to be a bug). */
	if (!(except_pending || breakpoint))
		cpu_state.e3reg[mD][uimm] = val;
}

INSTRUCTION (le_sw) {
	int old_cyc = 0;
	if (config.cpu.sbuf_len) old_cyc = runtime.sim.mem_cycles;
	
	orreg_t rD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t uimm = PARAM2;

	set_mem32(rD, cpu_state.e3reg[mA][uimm], &breakpoint);
	if (config.cpu.sbuf_len)
	{
		int t = runtime.sim.mem_cycles;
		runtime.sim.mem_cycles = old_cyc;
		sbuf_store (t - old_cyc);
	}
}

INSTRUCTION (le_sw4096) {
	int old_cyc = 0;
	if (config.cpu.sbuf_len) old_cyc = runtime.sim.mem_cycles;
	
	orreg_t rD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t uimm = PARAM2;
	rD += (E3_NUMWORDS-uimm-1) << 2;

	set_mem32(rD, cpu_state.e3reg[mA][uimm], &breakpoint);
	if (config.cpu.sbuf_len)
	{
		int t = runtime.sim.mem_cycles;
		runtime.sim.mem_cycles = old_cyc;
		sbuf_store (t - old_cyc);
	}
}

//Acceleration

INSTRUCTION (le_add) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_add(mpz_mD, mpz_mA, mpz_mB);
	//gmp_printf("le.add -> mD: %Zx\tmA: %Zx\tmB: %Zx\n", mpz_mD, mpz_mA, mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_and) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_and(mpz_mD, mpz_mA, mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mD);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_not) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	e3_not(mD, mA, reglen_bit);
	/*mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, reglen_bit);

	mpz_com(mpz_mD, mpz_mA);
	mpz_clear(mpz_mA);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);*/
}

INSTRUCTION (le_or) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_ior(mpz_mD, mpz_mA, mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_cmov) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	if (mpz_cmp_ui(mpz_mD, 0) != 0)
		e3_set_mpz(mpz_mD, mA, reglen_bit);
	else
		e3_set_mpz(mpz_mD, mB, reglen_bit);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_dec) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	mpz_sub_ui(mpz_mD, mpz_mD, 1);
	
	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_divs) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		unsigned signA = e3_get_sign(mA, reglen_bit);
		unsigned signB = e3_get_sign(mB, reglen_bit);

		mpz_t mpz_mD, mpz_mA;
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, reglen_bit);

		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);

		mpz_tdiv_q(mpz_mD, mpz_mA, mpz_mB);

		if (signA ^ signB) e3_twos_complement(mpz_mD, reglen_bit);
		e3_set_e3reg(mD, mpz_mD, reglen_bit);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, reglen_bit);
	}
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_divu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA, mpz_mB;
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, reglen_bit);

		mpz_tdiv_q(mpz_mD, mpz_mA, mpz_mB);

		e3_set_e3reg(mD, mpz_mD, reglen_bit);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, reglen_bit);
	}
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_eq) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) == 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_xor) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_xor(mpz_mD, mpz_mA, mpz_mB);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_pows) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);

	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_limit;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mpz_limit, 1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	mpz_mul_2exp(mpz_limit, mpz_limit, reglen_bit);

	if (signA) e3_twos_complement(mpz_mA, reglen_bit);

	mpz_powm(mpz_mD, mpz_mA, mpz_mB, mpz_limit);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_limit);

	if (signA) e3_twos_complement(mpz_mD, reglen_bit);
	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_powu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mpz_limit;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mpz_limit, 1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	mpz_mul_2exp(mpz_limit, mpz_limit, reglen_bit);

	mpz_powm(mpz_mD, mpz_mA, mpz_mB, mpz_limit);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_limit);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_ff1) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);

	unsigned pos = e3_ff1(mA, reglen_bit);
	mpz_set_ui(mpz_mD, pos);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);	
}

INSTRUCTION (le_fl1) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);

	unsigned pos = e3_fl1(mA, reglen_bit);
	mpz_set_ui(mpz_mD, pos);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);	
}

INSTRUCTION (le_gcd) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mE = (mD + 1) % E3_NUMREGS;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mE, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mE);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_gcd(mpz_mD, mpz_mA, mpz_mB);
	mpz_invert(mpz_mE, mpz_mA, mpz_mB);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	e3_set_e3reg(mE, mpz_mE, reglen_bit);
	mpz_clear(mpz_mD);
	mpz_clear(mpz_mE);
}

INSTRUCTION (le_ges) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);
	//printf("le_ges -> signA: %u\tsignB: %u\treglen_bit: %u\n", signA, signB, reglen_bit);
	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	int signDiff = signB - signA;
	if (signDiff > 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff < 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		mpz_t mpz_mA, mpz_mB;
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);
		
		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);

		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) <= 0) : (mpz_cmp(mpz_mA, mpz_mB) >= 0));
		mpz_set_ui(mpz_mD, cmp);
		//gmp_printf("le_ges -> mpz_mD: %Zx\tmpz_mA: %Zx\tmpz_mB: %Zx\n", mpz_mD, mpz_mA, mpz_mB);
		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_geu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) >= 0);
	mpz_set_ui(mpz_mD, cmp);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_gts) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	int signDiff = signB - signA;
	if (signDiff > 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff < 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		mpz_t mpz_mA, mpz_mB;
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);

		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);

		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) < 0) : (mpz_cmp(mpz_mA, mpz_mB) > 0));
		mpz_set_ui(mpz_mD, cmp);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_gtu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) > 0);
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_inc) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	mpz_add_ui(mpz_mD, mpz_mD, 1);
	
	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_les) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	int signDiff = signB - signA;
	if (signDiff < 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff > 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		mpz_t mpz_mA, mpz_mB;
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);

		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);

		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) >= 0) : (mpz_cmp(mpz_mA, mpz_mB) <= 0));
		mpz_set_ui(mpz_mD, cmp);
		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_leu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) <= 0);
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_lts) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD;
	mpz_init(mpz_mD);
	int signDiff = signB - signA;
	if (signDiff < 0) mpz_init_set_ui(mpz_mD, 1);
	else if (signDiff > 0) mpz_init_set_ui(mpz_mD, 0);
	else
	{
		mpz_t mpz_mA, mpz_mB;
		mpz_init(mpz_mA);
		mpz_init(mpz_mB);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		e3_set_mpz(mpz_mB, mB, reglen_bit);

		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);

		unsigned cmp = (signA ? (mpz_cmp(mpz_mA, mpz_mB) > 0) : (mpz_cmp(mpz_mA, mpz_mB) < 0));
		mpz_set_ui(mpz_mD, cmp);
		mpz_clear(mpz_mA);
		mpz_clear(mpz_mB);
	}

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_ltu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) < 0);
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_land) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp_ui(mpz_mA, 0) && mpz_cmp_ui(mpz_mB, 0));
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_lnot) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;

	mpz_t mpz_mD, mpz_mA;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	e3_set_mpz(mpz_mA, mA, reglen_bit);

	unsigned cmp = !mpz_cmp_ui(mpz_mA, 0);
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_lor) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp_ui(mpz_mA, 0) || mpz_cmp_ui(mpz_mB, 0));
	mpz_set_ui(mpz_mD, cmp);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_mods) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA;

		unsigned signA = e3_get_sign(mA, reglen_bit);
		unsigned signB = e3_get_sign(mB, reglen_bit);

		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, reglen_bit);
		
		if (signA) e3_twos_complement(mpz_mA, reglen_bit);
		if (signB) e3_twos_complement(mpz_mB, reglen_bit);
		mpz_mod(mpz_mD, mpz_mA, mpz_mB);
		if (signA) e3_twos_complement(mpz_mD, reglen_bit);
		e3_set_e3reg(mD, mpz_mD, reglen_bit);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, reglen_bit);
	}

	mpz_clear(mpz_mB);
}

INSTRUCTION (le_modu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mB;
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (mpz_cmp_ui(mpz_mB, 0))
	{
		mpz_t mpz_mD, mpz_mA;
		
		mpz_init(mpz_mD);
		mpz_init(mpz_mA);
		e3_set_mpz(mpz_mA, mA, reglen_bit);

		mpz_mod(mpz_mD, mpz_mA, mpz_mB);

		e3_set_e3reg(mD, mpz_mD, reglen_bit);

		mpz_clear(mpz_mA);
		mpz_clear(mpz_mD);
	}
	else
	{
		e3_rand(mD, reglen_bit);
	}
	
	mpz_clear(mpz_mB);
}

INSTRUCTION (le_macs) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	if (signA) e3_twos_complement(mpz_mA, reglen_bit);
	if (signB) e3_twos_complement(mpz_mB, reglen_bit);

	//mpz_addmul(mpz_mD, mpz_mA, mpz_mB);
	mpz_mul(mpz_mA, mpz_mA, mpz_mB);

	if (signA ^ signB) e3_twos_complement(mpz_mA, reglen_bit);

	mpz_add(mpz_mD, mpz_mD, mpz_mA);
	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_macu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_msbs) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	if (signA) e3_twos_complement(mpz_mA, reglen_bit);
	if (signB) e3_twos_complement(mpz_mB, reglen_bit);

	mpz_mul(mpz_mA, mpz_mA, mpz_mB);

	if (!(signA ^ signB)) e3_twos_complement(mpz_mA, reglen_bit);

	mpz_add(mpz_mD, mpz_mD, mpz_mA);
	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_msbu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);
	e3_set_mpz(mpz_mD, mD, reglen_bit);

	mpz_mul(mpz_mA, mpz_mA, mpz_mB);
	e3_twos_complement(mpz_mA, reglen_bit);
	mpz_add(mpz_mD, mpz_mD, mpz_mA);
	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_muls) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);
	unsigned signB = e3_get_sign(mB, reglen_bit);

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (signA) e3_twos_complement(mpz_mA, reglen_bit);
	if (signB) e3_twos_complement(mpz_mB, reglen_bit);

	mpz_mul(mpz_mD, mpz_mA, mpz_mB);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	if (signA ^ signB) e3_twos_complement(mpz_mD, reglen_bit);
	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_mulu) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	mpz_mul(mpz_mD, mpz_mA, mpz_mB);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_ne) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	unsigned cmp = (mpz_cmp(mpz_mA, mpz_mB) != 0);
	mpz_set_ui(mpz_mD, cmp);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_rand) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();
	orreg_t mD = PARAM0;
	
	e3_rand(mD, reglen_bit);
}

INSTRUCTION (le_ror) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, reglen_bit-1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	
	// rotating the bits lower than the shift to the MSB positions
	mpz_set_ui(mask, 1);
	mpz_mul_2exp(mask, mask, shift);
	mpz_sub_ui(mask, mask, 1);
	mpz_and(mpz_mB, mpz_mA, mask);
	mpz_mul_2exp(mpz_mB, mpz_mB, reglen_bit-shift);

	// shifting the higher bits right
	mpz_tdiv_q_2exp(mpz_mA, mpz_mA, shift);

	// merging parts
	mpz_xor(mpz_mD, mpz_mA, mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_sll) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, reglen_bit-1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_mul_2exp(mpz_mD, mpz_mA, shift);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_sra) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	unsigned signA = e3_get_sign(mA, reglen_bit);

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, reglen_bit-1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	if (signA) e3_twos_complement(mpz_mA, reglen_bit);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_tdiv_q_2exp(mpz_mD, mpz_mA, shift);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);
	e3_extend_sign(mD, signA, reglen_bit);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_srl) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB, mask;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	mpz_init_set_ui(mask, reglen_bit-1);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	// selecting only the lower bits of mB
	mpz_and(mpz_mB, mpz_mB, mask);
	unsigned shift = mpz_get_ui(mpz_mB);
	mpz_tdiv_q_2exp(mpz_mD, mpz_mA, shift);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mask);
	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

INSTRUCTION (le_sub) {
	unsigned reglen_bit = e3_get_effective_decrypted_size();

	orreg_t mD = PARAM0;
	orreg_t mA = PARAM1;
	orreg_t mB = PARAM2;

	mpz_t mpz_mD, mpz_mA, mpz_mB;
	mpz_init(mpz_mD);
	mpz_init(mpz_mA);
	mpz_init(mpz_mB);
	e3_set_mpz(mpz_mA, mA, reglen_bit);
	e3_set_mpz(mpz_mB, mB, reglen_bit);

	e3_twos_complement(mpz_mB, reglen_bit);
	mpz_add(mpz_mD, mpz_mA, mpz_mB);

	e3_set_e3reg(mD, mpz_mD, reglen_bit);

	mpz_clear(mpz_mA);
	mpz_clear(mpz_mB);
	mpz_clear(mpz_mD);
}

/** MoMA end **/

