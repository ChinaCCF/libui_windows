// 4 december 2014
#include "uipriv_windows.hpp"

typedef std::vector<uint8_t> byteArray;

static std::map<uint8_t *, byteArray *> g_mem_heap;
static std::map<byteArray *, const char *> g_mem_types;

void init_Alloc(void)
{
	// do nothing
}

void uninit_Alloc(void)
{
	std::ostringstream oss;
	std::string ossstr;		// keep alive, just to be safe

	if (g_mem_heap.size() == 0)
		return;
	for (const auto &alloc : g_mem_heap) 
		oss << (void *) (alloc.first) << " " << g_mem_types[alloc.second] << "\n";
	ossstr = oss.str();
	uiprivUserBug("Some data was leaked; either you left a uiControl lying around or there's a bug in libui itself. Leaked data:\n%s", ossstr.c_str());
}

#define rawBytes(pa) (&((*pa)[0]))

void *libui_alloc(size_t size, const char *type)
{
	byteArray *out;

	out = new byteArray(size, 0);
	g_mem_heap[rawBytes(out)] = out;
	g_mem_types[out] = type;
	return rawBytes(out);
}

void *libui_realloc(void *_p, size_t size, const char *type)
{
	uint8_t *p = (uint8_t *) _p;
	byteArray *arr;

	if (p == NULL)
		return libui_alloc(size, type);
	arr = g_mem_heap[p];
	// TODO does this fill in?
	arr->resize(size, 0);
	g_mem_heap.erase(p);
	g_mem_heap[rawBytes(arr)] = arr;
	return rawBytes(arr);
}

void libui_free(void *_p)
{
	uint8_t *p = (uint8_t *) _p;

	if (p == NULL)
		uiprivImplBug("attempt to uiprivFree(NULL)");
	g_mem_types.erase(g_mem_heap[p]);
	delete g_mem_heap[p];
	g_mem_heap.erase(p);
}
