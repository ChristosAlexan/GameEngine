#include "AdapterReader.h"

std::vector<AdapterData> AdapterReader::adapters;

std::vector<AdapterData> AdapterReader::GetAdapters()
{
	if (adapters.size() > 0) //If already initialized
		return adapters;

	Microsoft::WRL::ComPtr<IDXGIFactory> pFactory;

	// Create a DXGIFactory object.
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(pFactory.GetAddressOf()));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create DXGIFactory for enumerating adapters.");
		exit(-1);
	}

	IDXGIAdapter* pAdapter;
	UINT index = 0;
	while (SUCCEEDED(pFactory->EnumAdapters(index, &pAdapter)))
	{
		
		adapters.push_back(AdapterData(pAdapter));
		index += 1;
	}
	

	return adapters;
}

void AdapterReader::SetAdapterOutput(AdapterData& adapter, IDXGIOutput* adapterOutput)
{
	HRESULT hr = adapter.pAdapter->EnumOutputs(0, &adapterOutput);
}

AdapterData::AdapterData(IDXGIAdapter* pAdapter)
{
	this->pAdapter = pAdapter;
	HRESULT hr = pAdapter->GetDesc(&this->description);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Get Description for IDXGIAdapter.");
	}
}