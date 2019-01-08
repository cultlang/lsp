#include "lsp/common.h"

#include "all.h"

#include "lisp/library/system/prelude.h"
#include "lisp/semantics/cult/calling.h"

using namespace craft;
using namespace craft::lisp;
using namespace craft::types;

namespace _impl {
	//https://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12759/URI-Encoding-and-Decoding.htm
	const char SAFE[256] =
	{
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

		/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
		/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

		/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

		/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
	};

	const char HEX2DEC[256] =
	{
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

		/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	};

	std::string UriEncode(const std::string & sSrc)
	{
		const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
		const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
		const int SRC_LEN = sSrc.length();
		unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
		unsigned char * pEnd = pStart;
		const unsigned char * const SRC_END = pSrc + SRC_LEN;

		for (; pSrc < SRC_END; ++pSrc)
		{
			if (SAFE[*pSrc])
				*pEnd++ = *pSrc;
			else
			{
				// escape this char
				*pEnd++ = '%';
				*pEnd++ = DEC2HEX[*pSrc >> 4];
				*pEnd++ = DEC2HEX[*pSrc & 0x0F];
			}
		}

		std::string sResult((char *)pStart, (char *)pEnd);
		delete[] pStart;
		return sResult;
	}

	std::string UriDecode(const std::string & sSrc)
	{
		// Note from RFC1630: "Sequences which start with a percent
		// sign but are not followed by two hexadecimal characters
		// (0-9, A-F) are reserved for future extension"

		const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
		const int SRC_LEN = sSrc.length();
		const unsigned char * const SRC_END = pSrc + SRC_LEN;
		// last decodable '%' 
		const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

		char * const pStart = new char[SRC_LEN];
		char * pEnd = pStart;

		while (pSrc < SRC_LAST_DEC)
		{
			if (*pSrc == '%')
			{
				char dec1, dec2;
				if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
					&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
				{
					*pEnd++ = (dec1 << 4) + dec2;
					pSrc += 3;
					continue;
				}
			}

			*pEnd++ = *pSrc++;
		}

		// the last 2- chars
		while (pSrc < SRC_END)
			*pEnd++ = *pSrc++;

		std::string sResult(pStart, pEnd);
		delete[] pStart;
		return sResult;
	}
}

void cultlang::lsp::make_bindings(craft::instance<craft::lisp::Module> ret)
{
	auto sem = ret->require<lisp::CultSemantics>();

	sem->builtin_implementMultiMethod("lsp/safe-require", [](instance<std::string> file) {
		// try
		// {
		// 	ScopeManipulation::Require(fmt::format("file:{0}", *file));
		// }
		// catch(stdext::exception e)
		// {
		// 	return instance<std::string>::make(e.what());
		// }
		// catch (std::exception e)
		// {
		// 	return instance<std::string>::make(e.what());
		// }

		// return instance<std::string>::make("Module Loaded");
	});
	sem->builtin_implementMultiMethod("lsp/uri/scheme", [](instance<std::string> s) {
		std::smatch sm;
		std::regex_match(*s, sm, std::regex("(\\w+)://"));
		if (sm.size()) return instance<std::string>::make(sm[1]);
		else return instance<std::string>::make("");
	});
	sem->builtin_implementMultiMethod("lsp/uri/filepath", [](instance<std::string> s) {
		return instance<std::string>::make(std::regex_replace(*s, std::regex("\\w+:///"), ""));
	});

	sem->builtin_implementMultiMethod("lsp/uri/encode", [](instance<std::string> s) {
		return instance<std::string>::make(_impl::UriEncode(*s));
	});

	sem->builtin_implementMultiMethod("lsp/uri/decode", [](instance<std::string> s) {
		return instance<std::string>::make(_impl::UriDecode(*s));	
	});

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
		// auto ns = Execution::getCurrent()->getNamespace();
		// auto d = ns->symbolStore.expensivedump();
		// auto res = instance<lisp::library::List>::make();
		// for (auto i : d) {
		// 	auto m = instance<lisp::library::Map>::make();
		// 	m->insert(instance<std::string>::make("label"), i);
		// 	m->insert(instance<std::string>::make("kind"), instance<int32_t>::make(3));
		// 	m->insert(instance<std::string>::make("detail"), i);

		// 	res->push(m);
		// }
		return;
	});

	return;
}

BuiltinModuleDescription cultlang::lsp::BuiltinLsp("extensions/lsp", cultlang::lsp::make_bindings);
#include "types/dll_entry.inc"
