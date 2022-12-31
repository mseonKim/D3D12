#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include <random>
#include <chrono>

#ifdef CH13_EX1

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

struct Data
{
    XMFLOAT3 v;
};

const unsigned numElements = 64;


class CH13Exercise1App : public D3DApp
{
public:
    CH13Exercise1App(HINSTANCE hInstance);
    CH13Exercise1App(const CH13Exercise1App& rhs) = delete;
    CH13Exercise1App& operator=(const CH13Exercise1App& rhs) = delete;
    ~CH13Exercise1App();

    virtual bool Initialize()override;

    void DoComputeWork();

private:
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;


    void BuildRootSignature();
    void BuildResources();
    void BuildShadersAndInputLayout();
    void BuildPSO();

private:

    UINT mCbvSrvUavDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

    ComPtr<ID3D12Resource> mInputBuffer = nullptr;
    ComPtr<ID3D12Resource> mInputUploadBuffer = nullptr;
    ComPtr<ID3D12Resource> mOutputBuffer = nullptr;
    ComPtr<ID3D12Resource> mReadBackBuffer = nullptr;

    ComPtr<ID3DBlob> mComputeShader;
    ComPtr<ID3D12PipelineState> mPSO;
};

CH13Exercise1App::CH13Exercise1App(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

CH13Exercise1App::~CH13Exercise1App()
{
    if (md3dDevice != nullptr)
        FlushCommandQueue();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        CH13Exercise1App theApp(hInstance);
        if (!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

void CH13Exercise1App::Update(const GameTimer& gt)
{
}

bool CH13Exercise1App::Initialize()
{
    if (!D3DApp::Initialize())
        return false;

    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    BuildRootSignature();
    BuildResources();
    BuildShadersAndInputLayout();
    BuildPSO();

    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    DoComputeWork();

    return true;
}

void CH13Exercise1App::Draw(const GameTimer& gt)
{
}

void CH13Exercise1App::DoComputeWork()
{
    ThrowIfFailed(mDirectCmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    mCommandList->SetComputeRootSignature(mRootSignature.Get());
    mCommandList->SetComputeRootShaderResourceView(0, mInputBuffer->GetGPUVirtualAddress());
    mCommandList->SetComputeRootUnorderedAccessView(1, mOutputBuffer->GetGPUVirtualAddress());

    mCommandList->SetPipelineState(mPSO.Get());

    mCommandList->Dispatch(numElements, 1, 1);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutputBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE
    ));
    mCommandList->CopyResource(mReadBackBuffer.Get(), mOutputBuffer.Get());
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutputBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON
    ));

    ThrowIfFailed(mCommandList->Close());

    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();


    // Print result
    float* mappedData = nullptr;
    ThrowIfFailed(mReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));

    std::ofstream fout("results.txt");

    for (int i = 0; i < numElements; i++)
    {
        fout << "(" << mappedData[i] << ")" << std::endl;
    }

    mReadBackBuffer->Unmap(0, nullptr);
}


void CH13Exercise1App::BuildRootSignature()
{
    CD3DX12_ROOT_PARAMETER slotParameter[2];
    slotParameter[0].InitAsShaderResourceView(0);
    slotParameter[1].InitAsUnorderedAccessView(0);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())
    ));
}


void CH13Exercise1App::BuildResources()
{
    std::vector<Data> data(numElements);
    
    for (int i = 0; i < numElements; i++)
    {
        std::mt19937_64 rng1(i);
        std::uniform_int_distribution<__int64> dist1(1, 10);
        int r = dist1(rng1);
        data[i].v = XMFLOAT3(r, r, r);
    }

    const UINT64 byteSize = data.size() * sizeof(Data);

    mInputBuffer =  d3dUtil::CreateDefaultBuffer(
        md3dDevice.Get(),
        mCommandList.Get(),
        data.data(),
        byteSize,
        mInputUploadBuffer
    );

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&mOutputBuffer)));
    

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&mReadBackBuffer)));
}

void CH13Exercise1App::BuildShadersAndInputLayout()
{
    mComputeShader = d3dUtil::CompileShader(L"Shaders\\ComputeShaders\\ComputeShaderEx1.hlsl", nullptr, "main", "cs_5_0");
}

void CH13Exercise1App::BuildPSO()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC calcLengthPSO = {};
    calcLengthPSO.pRootSignature = mRootSignature.Get();
    calcLengthPSO.CS = {
        reinterpret_cast<BYTE*>(mComputeShader->GetBufferPointer()),
        mComputeShader->GetBufferSize()
    };
    calcLengthPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&calcLengthPSO, IID_PPV_ARGS(&mPSO)));
}
#endif