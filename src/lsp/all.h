#pragma once
#include "lsp/common.h"

namespace cultlang {
namespace lsp {
	CULTLANG_LSP_EXPORTED craft::instance<craft::lisp::Module> make_lsp_bindings(craft::instance<craft::lisp::Namespace> ns, craft::instance<> loader);
}}
