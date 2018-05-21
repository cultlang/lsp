#include "uv/common.h"

#include "all.h"

#include "lisp/library/system/prelude.h"
#include "lisp/semantics/cult/calling.h"

using namespace craft;
using namespace craft::lisp;
using namespace craft::types;
using namespace cultlang::uv;

instance<Module> cultlang::lsp::make_lsp_bindings(craft::instance<craft::lisp::Namespace> ns, craft::instance<> loader)
{
	auto ret = instance<Module>::make(ns, loader);
	auto sem = instance<CultSemantics>::make(ret);
	ret->builtin_setSemantics(sem);

	sem->builtin_implementMultiMethod("lsp/body",
		[](instance<std::string> s, instance<std::string> out)
	{
		if (!s->size())
		{
			return instance<bool>::make(false);
		}
		std::regex re("Content-Length: ([0-9]+)");
		std::smatch base_match;
		// No Content Lengths is a malformed input
		if (!std::regex_search(*s, base_match, re))
		{
			out->assign("");
			s->assign("");
			return instance<bool>::make(true);
		}
		size_t length = std::stol(base_match[1]);
		size_t ind = s->find("\r\n\r\n") + 4;
		*out = s->substr(ind, length);
		s->erase(0, ind + length);
		return instance<bool>::make(true);
	});

	sem->builtin_implementMultiMethod("lsp/makeResponse",
		[](instance<std::string> body)
	{
		size_t length = body->size();
		return instance<std::string>::make(
			fmt::format("Content-Type: application/vscode-jsonrpc; charset=utf-8\r\nContent-Length: {0}\r\n\r\n{1}", length, *body));
	});

	sem->builtin_implementMultiMethod("lsp/dumpsymbols",
		[]()
	{
		auto ns = Execution::getCurrent()->getNamespace();
		auto d = ns->symbolStore.expensivedump();
		auto res = instance<lisp::library::List>::make();
		for (auto i : d) {
			auto m = instance<lisp::library::Map>::make();
			m->insert(instance<std::string>::make("label"), i);
			m->insert(instance<std::string>::make("kind"), instance<int32_t>::make(3));
			m->insert(instance<std::string>::make("detail"), i);

			res->push(m);
		}
		return res;
	});

	return ret;
}

BuiltinModuleDescription cultlang::uv::BuiltinUv("cult/lsp", cultlang::lsp::make_lsp_bindings);
#include "types/dll_entry.inc"
