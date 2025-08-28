/*
 * Copyright (c) 2025 László Várady
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Orc.h>
#include <llvm-c/Transforms/PassBuilder.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define DEBUG 1

LLVMErrorRef thr_optimize(void *user_data, LLVMModuleRef mod) {
  LLVMPassBuilderOptionsRef options = LLVMCreatePassBuilderOptions();
  // TODO smaller faster set: function(instcombine), etc.
  LLVMErrorRef err = LLVMRunPasses(mod, "default<O2>", NULL, options);
  LLVMDisposePassBuilderOptions(options);
  return err;
}

LLVMErrorRef optimize(void *user_data, LLVMOrcThreadSafeModuleRef *thr_mod, LLVMOrcMaterializationResponsibilityRef mr)
{
  return LLVMOrcThreadSafeModuleWithModuleDo(*thr_mod, thr_optimize, user_data);
}

int main()
{
  // ---- init
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  LLVMContextRef ctx = LLVMContextCreate();
  LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("filterx::jit", ctx); // TODO use Itanium name mangling and namespaces
  LLVMBuilderRef ir = LLVMCreateBuilderInContext(ctx);

  // ---- IR

  LLVMTypeRef int32_type = LLVMInt32TypeInContext(ctx);
  LLVMTypeRef void_type = LLVMVoidTypeInContext(ctx);

  LLVMTypeRef fx_block_signature = LLVMFunctionType(int32_type, NULL, 0, false);
  LLVMValueRef fx_block = LLVMAddFunction(mod, "filterx::jit::block13592342", fx_block_signature); // TODO Itanium name mangling and namespaces
  LLVMBasicBlockRef fx_block_entry = LLVMAppendBasicBlockInContext(ctx, fx_block, "entry");

  LLVMPositionBuilderAtEnd(ir, fx_block_entry);
  LLVMBuildRet(ir, LLVMConstInt(int32_type, 13, false));

  // ---- IR verify

#ifdef DEBUG
  LLVMVerifyFunction(fx_block, LLVMAbortProcessAction);
  char* error = NULL;
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);
#endif

  char* irCode = LLVMPrintModuleToString(mod);
  printf("%s", irCode);
  LLVMDisposeMessage(irCode);

  // ---- JIT

  LLVMOrcLLJITRef jit;
  LLVMCantFail(LLVMOrcCreateLLJIT(&jit, LLVMOrcCreateLLJITBuilder()));

  // JIT-time optimizations
  {
    LLVMOrcIRTransformLayerRef transform = LLVMOrcLLJITGetIRTransformLayer(jit);
    LLVMOrcIRTransformLayerSetTransform(transform, optimize, NULL);
  }

  // TODO LLVMOrcThreadSafeContextGetContext() vs LLVMOrcCreateNewThreadSafeContextFromLLVMContext() compat
  LLVMOrcThreadSafeContextRef thr_ctx = LLVMOrcCreateNewThreadSafeContext();
  LLVMOrcThreadSafeModuleRef thr_mod = LLVMOrcCreateNewThreadSafeModule(mod, thr_ctx);
  LLVMOrcDisposeThreadSafeContext(thr_ctx); // shared

  {
    LLVMOrcJITDylibRef jit_dylib = LLVMOrcLLJITGetMainJITDylib(jit);
    LLVMErrorRef err;
    if ((err = LLVMOrcLLJITAddLLVMIRModule(jit, jit_dylib, thr_mod))) {
      LLVMOrcDisposeThreadSafeModule(thr_mod);
      goto bye;
    }
  }

  LLVMOrcJITTargetAddress fx_block_addr;
  {
    LLVMErrorRef err;
    if ((err = LLVMOrcLLJITLookup(jit, &fx_block_addr, "filterx::jit::block13592342"))) {
      goto bye;
    }
  }

  int32_t (*block13592342)(void) = (int32_t(*)(void)) fx_block_addr;
  int32_t test_result = block13592342();
  printf("%d\n", test_result);

  // ---- deinit
bye:
  LLVMOrcDisposeLLJIT(jit);

  LLVMDisposeBuilder(ir);
  LLVMContextDispose(ctx);

  LLVMShutdown();
  return 0;
}
