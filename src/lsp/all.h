#pragma once
#include "lsp/common.h"

namespace cultlang {
namespace lsp {
	extern craft::lisp::BuiltinModuleDescription BuiltinLsp;
	CULTLANG_LSP_EXPORTED void make_bindings(craft::instance<craft::lisp::Module> ret);
}}
