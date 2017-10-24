/* simpl32-defs.h -- Definitions for the simple execution model

   Copyright (C) 1999 Damjan Lampret, lampret@opencores.org
   Copyright (C) 2008 Embecosm Limited

   Contributed by Damjan Lampret (lampret@opencores.org).
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Or1ksim, the OpenRISC 1000 Architectural Simulator.
   This file is also part of or1k_gen_isa, GDB and GAS.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* This program is commented throughout in a fashion suitable for processing
   with Doxygen. */


#ifndef SIMPLE32_DEFS__H
#define SIMPLE32_DEFS__H

/* Package includes */
#include "abstract.h"

extern void l_invalid PARAMS((struct iqueue_entry *));
extern void l_sfne PARAMS((struct iqueue_entry *));
extern void l_bf PARAMS((struct iqueue_entry *));
extern void l_add PARAMS((struct iqueue_entry *));
extern void l_addc PARAMS((struct iqueue_entry *));
extern void l_swa PARAMS((struct iqueue_entry *));
extern void l_sw PARAMS((struct iqueue_entry *));
extern void l_sb PARAMS((struct iqueue_entry *));
extern void l_sh PARAMS((struct iqueue_entry *));
extern void l_lwa PARAMS((struct iqueue_entry *));
extern void l_lwz PARAMS((struct iqueue_entry *));
extern void l_lbs PARAMS((struct iqueue_entry *));
extern void l_lbz PARAMS((struct iqueue_entry *));
extern void l_lhs PARAMS((struct iqueue_entry *));
extern void l_lhz PARAMS((struct iqueue_entry *));
extern void l_movhi PARAMS((struct iqueue_entry *));
extern void l_and PARAMS((struct iqueue_entry *));
extern void l_or PARAMS((struct iqueue_entry *));
extern void l_xor PARAMS((struct iqueue_entry *));
extern void l_sub PARAMS((struct iqueue_entry *));
extern void l_mul PARAMS((struct iqueue_entry *));
extern void l_div PARAMS((struct iqueue_entry *));
extern void l_divu PARAMS((struct iqueue_entry *));
extern void l_sll PARAMS((struct iqueue_entry *));
extern void l_sra PARAMS((struct iqueue_entry *));
extern void l_srl PARAMS((struct iqueue_entry *));
extern void l_j PARAMS((struct iqueue_entry *));
extern void l_jal PARAMS((struct iqueue_entry *));
extern void l_jalr PARAMS((struct iqueue_entry *));
extern void l_jr PARAMS((struct iqueue_entry *));
extern void l_rfe PARAMS((struct iqueue_entry *));
extern void l_nop PARAMS((struct iqueue_entry *));
extern void l_bnf PARAMS((struct iqueue_entry *));
extern void l_sfeq PARAMS((struct iqueue_entry *));
extern void l_sfgts PARAMS((struct iqueue_entry *));
extern void l_sfges PARAMS((struct iqueue_entry *));
extern void l_sflts PARAMS((struct iqueue_entry *));
extern void l_sfles PARAMS((struct iqueue_entry *));
extern void l_sfgtu PARAMS((struct iqueue_entry *));
extern void l_sfgeu PARAMS()(struct iqueue_entry *);
extern void l_sfltu PARAMS((struct iqueue_entry *));
extern void l_sfleu PARAMS((struct iqueue_entry *));
extern void l_extbs PARAMS((struct iqueue_entry *));
extern void l_extbz PARAMS((struct iqueue_entry *));
extern void l_exths PARAMS((struct iqueue_entry *));
extern void l_exthz PARAMS((struct iqueue_entry *));
extern void l_extws PARAMS((struct iqueue_entry *));
extern void l_extwz PARAMS((struct iqueue_entry *));
extern void l_mtspr PARAMS((struct iqueue_entry *));
extern void l_mfspr PARAMS((struct iqueue_entry *));
extern void l_sys PARAMS((struct iqueue_entry *));
extern void l_trap PARAMS((struct iqueue_entry *)); /* CZ 21/06/01 */
extern void l_macrc PARAMS((struct iqueue_entry *));
extern void l_mac PARAMS((struct iqueue_entry *));
extern void l_msb PARAMS((struct iqueue_entry *));
extern void l_cmov PARAMS ((struct iqueue_entry *));
extern void l_ff1 PARAMS ((struct iqueue_entry *));
extern void l_cust1 PARAMS ((struct iqueue_entry *));
extern void l_cust2 PARAMS ((struct iqueue_entry *));
extern void l_cust3 PARAMS ((struct iqueue_entry *));
extern void l_cust4 PARAMS ((struct iqueue_entry *));
extern void lf_add_s PARAMS ((struct iqueue_entry *));
extern void lf_div_s PARAMS ((struct iqueue_entry *));
extern void lf_ftoi_s PARAMS ((struct iqueue_entry *));
extern void lf_itof_s PARAMS ((struct iqueue_entry *));
extern void lf_madd_s PARAMS ((struct iqueue_entry *));
extern void lf_mul_s PARAMS ((struct iqueue_entry *));
extern void lf_rem_s PARAMS ((struct iqueue_entry *));
extern void lf_sfeq_s PARAMS ((struct iqueue_entry *));
extern void lf_sfge_s PARAMS ((struct iqueue_entry *));
extern void lf_sfgt_s PARAMS ((struct iqueue_entry *));
extern void lf_sfle_s PARAMS ((struct iqueue_entry *));
extern void lf_sflt_s PARAMS ((struct iqueue_entry *));
extern void lf_sfne_s PARAMS ((struct iqueue_entry *));
extern void lf_sub_s PARAMS((struct iqueue_entry *));
extern void l_none PARAMS((struct iqueue_entry *));

/** MoMA begin **/
extern void l_mod PARAMS((struct iqueue_entry *));

// Secure Computation


// Other
extern void le_enc PARAMS((struct iqueue_entry *));
extern void le_mafdtspr PARAMS((struct iqueue_entry *));
extern void le_mabdtspr PARAMS((struct iqueue_entry *));
extern void le_mabfdtspr PARAMS((struct iqueue_entry *));
extern void le_maefspr PARAMS((struct iqueue_entry *));
extern void le_mfftsk PARAMS((struct iqueue_entry *));
extern void le_mfbtsk PARAMS((struct iqueue_entry *));
extern void le_mfer PARAMS((struct iqueue_entry *));
extern void le_mter PARAMS((struct iqueue_entry *));
extern void le_mfspr PARAMS((struct iqueue_entry *));
extern void le_mtspr PARAMS((struct iqueue_entry *));
extern void le_sfbusy PARAMS((struct iqueue_entry *));
extern void le_lw PARAMS((struct iqueue_entry *));
extern void le_sw PARAMS((struct iqueue_entry *));


// Acceleration
extern void le_add PARAMS((struct iqueue_entry *));
extern void le_and PARAMS((struct iqueue_entry *));
extern void le_not PARAMS((struct iqueue_entry *));
extern void le_or PARAMS((struct iqueue_entry *));
extern void le_cmov PARAMS((struct iqueue_entry *));
extern void le_dec PARAMS((struct iqueue_entry *));
extern void le_divs PARAMS((struct iqueue_entry *));
extern void le_divu PARAMS((struct iqueue_entry *));
extern void le_eq PARAMS((struct iqueue_entry *));
extern void le_xor PARAMS((struct iqueue_entry *));
extern void le_pows PARAMS((struct iqueue_entry *));
extern void le_powu PARAMS((struct iqueue_entry *));
extern void le_ff1 PARAMS((struct iqueue_entry *));
extern void le_fl1 PARAMS((struct iqueue_entry *));
extern void le_gcd PARAMS((struct iqueue_entry *));
extern void le_ges PARAMS((struct iqueue_entry *));
extern void le_geu PARAMS((struct iqueue_entry *));
extern void le_gts PARAMS((struct iqueue_entry *));
extern void le_gtu PARAMS((struct iqueue_entry *));
extern void le_inc PARAMS((struct iqueue_entry *));
extern void le_les PARAMS((struct iqueue_entry *));
extern void le_leu PARAMS((struct iqueue_entry *));
extern void le_lts PARAMS((struct iqueue_entry *));
extern void le_ltu PARAMS((struct iqueue_entry *));
extern void le_land PARAMS((struct iqueue_entry *));
extern void le_lnot PARAMS((struct iqueue_entry *));
extern void le_lor PARAMS((struct iqueue_entry *));
extern void le_mods PARAMS((struct iqueue_entry *));
extern void le_modu PARAMS((struct iqueue_entry *));
extern void le_macs PARAMS((struct iqueue_entry *));
extern void le_macu PARAMS((struct iqueue_entry *));
extern void le_msbs PARAMS((struct iqueue_entry *));
extern void le_msbu PARAMS((struct iqueue_entry *));
extern void le_muls PARAMS((struct iqueue_entry *));
extern void le_mulu PARAMS((struct iqueue_entry *));
extern void le_ne PARAMS((struct iqueue_entry *));
extern void le_rand PARAMS((struct iqueue_entry *));
extern void le_ror PARAMS((struct iqueue_entry *));
extern void le_sll PARAMS((struct iqueue_entry *));
extern void le_sra PARAMS((struct iqueue_entry *));
extern void le_srl PARAMS((struct iqueue_entry *));
extern void le_sub PARAMS((struct iqueue_entry *));

/*
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
extern void le_ PARAMS((struct iqueue_entry *));
*/
/** MoMA end **/

#endif	/* SIMPLE32_DEFS__H */
