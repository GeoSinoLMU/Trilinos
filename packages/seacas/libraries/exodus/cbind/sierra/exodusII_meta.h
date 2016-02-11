/*! \file exodus_meta.h
 *
 * NOTE: This version is a hard-wired version for Sierra
 *       We "know" what options sierra has turned on in
 *       exodus and netcdf, so instead of generating this
 *       file by querying the build environment and netcdf,
 *       we just hard-wire the values...
 * 
 * Meta information for libexodus which can be used by other packages
 *  which depend on libexodus.
 *
 * This file is automatically generated by the build system
 * at configure time, and contains information related to
 * how libexodus was built.  It will not be required to
 * include this file unless you want to probe the capabilities
 * of libexodus. This should ideally only happen when configuring
 * a project which depends on libexodus.  At configure time,
 * the dependant project can set its own macros which can be used
 * in conditionals.
 */

/*
 * Copyright (c) 2005 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.  
 * 
 *     * Neither the name of Sandia Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef EXODUS_META_H
#define EXODUS_META_H

#define EX_HAS_NETCDF4   1  /*!< netcdf-4/hdf5 support. */
#define EX_HAS_PNETCDF   1  /*!< pnetcdf support (parallel IO for netcdf-3). */
#define EX_HAS_PARALLEL  1  /*!< parallel support. */

#endif
