#pragma once
#include "lsp/common.h"

namespace cultlang {
namespace lsp {
	extern craft::lisp::BuiltinModuleDescription BuiltinLsp;
	CULTLANG_LSP_EXPORTED craft::instance<craft::lisp::Module> make_lsp_bindings(craft::instance<craft::lisp::Namespace> ns, craft::instance<> loader);
}}
