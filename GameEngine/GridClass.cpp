#include "GridClass.h"

GridClass::GridClass()
{
	acceptedRadius = 1.3f;
	showGrid = false;
	bCreateOnInit = true;

	if (bCreateOnInit)
		bInitGrid = true;
	else
		bInitGrid = false;

	//bounds = DirectX::XMFLOAT3(11.3f, 1.0f, 5.0f);
	bounds = DirectX::XMFLOAT3(34.2f, 6.0f, 39.3f);
	showMode = 0;

	//bGridInitStage = false;
	bRayCastStage = false;
	bCreatePathStage = false;
}

void GridClass::InitializeBoundsVolume(ID3D11Device* device)
{
	cubeBoundsVolume.Initialize(device);
}

void GridClass::Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& device, Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext, DirectX::XMMATRIX transformMatrix, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	_device = device;
	_deviceContext = deviceContext;
	_transformMatrix = transformMatrix;
	_cb_vs_vertexshader = cb_vs_vertexshader;
}

void GridClass::DrawGrid(ID3D11DeviceContext* deviceContext, Camera& camera, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, std::vector<NodeClass>& validNodes)
{
	//if (showGrid)
	//{
		for (int i = 0; i < validNodes.size(); ++i)
		{
			validNodes[i].DrawNode(deviceContext, camera, cb_vs_vertexshader,showMode);
		}


	//}
}

void GridClass::DrawBounds(ID3D11DeviceContext* deviceContext, Camera& camera, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	cubeBoundsVolume.Draw(deviceContext, camera, cb_vs_vertexshader);
}

void GridClass::FindNeighbours(NodeClass& currentNode, std::vector<NodeClass>& validNodes)
{
	for (auto& node : validNodes)
	{
		//Use abs values to calculate both negative and positive bounds
		if (std::abs(currentNode.pos.x - node.pos.x) <= 1 &&
			std::abs(currentNode.pos.y - node.pos.y) <= 1 &&
			std::abs(currentNode.pos.z - node.pos.z) <= 1 &&
			&currentNode != &node)
		{
			currentNode.neighbours.push_back(&node);
		}
	}
}

void GridClass::CreatePathGrid(std::vector<NodeClass>& validNodes)
{
	for (int i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].isValidPath)
		{
			validNodes.push_back(nodes[i]);
		}
	}
	//OutputDebugStringA(("SIZE = " + std::to_string(validNodes.size()) + "\n").c_str());

	if (!validNodes.empty())
	{
		for (int i = 0; i < validNodes.size(); ++i)
		{

			if (validNodes[i].isValidPath)
				FindNeighbours(validNodes[i], validNodes);
		}
	}
}

void GridClass::SetupGridBounds()
{
	cubeBoundsVolume.scale = bounds;
}

void GridClass::DrawGUI()
{
	ImGui::DragFloat3("Bounds", &bounds.x, 0.1f);
	ImGui::InputInt("mode", &showMode);
	ImGui::DragFloat("accepted radius", &acceptedRadius, 0.02f);
}

void GridClass::InitGrid()
{
	//Compute bounds in both positive and negative axis
	int totalNodes = (2 * bounds.x + 1) * (2 * bounds.y + 1) * (2 * bounds.z + 1);

	for (int i = 0; i < totalNodes; ++i)
	{
		//modulo ensures that index cycles within - and + bounds
		int x = -(int)bounds.x + (i % (2 * (int)bounds.x + 1));
		int y = -(int)bounds.y + ((i / (2 * (int)bounds.x + 1)) % (2 * (int)bounds.y + 1));
		int z = -(int)bounds.z + (i / ((2 * (int)bounds.x + 1) * (2 * (int)bounds.y + 1)));

		nodes.push_back(NodeClass());
		nodes.back().pos = DirectX::XMFLOAT3(pos.x - x, pos.y - y, pos.z - z);
		nodes.back().Initialize(_device, _deviceContext, _transformMatrix, _cb_vs_vertexshader, true);
	}
}
