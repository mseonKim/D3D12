/*
 * Compute Shader�� ���̰� [1,10] ������ 3���� ���� 64���� ���� ������ ���۸� �Է¹޾Ƽ� �� ���͵��� ���̸� ����ϰ�,
 * ��� ����� �ϳ��� �ε��Ҽ��� ���ۿ� ����Ѵ�.
 * �������α׷��� �� ����� �ý��� �޸𸮿� �����ؼ� ���Ͽ� �����Ѵ�.
 * �����ϰ� ���Ͽ� ��ϵ� ��� ���̰� ������ [1,10] �������� �����϶�.
 */

#define N 64

struct Data
{
	float3 v;
};

StructuredBuffer<Data> gInput			: register(t0);
RWStructuredBuffer<float> gOutput	: register(u0);


[numthreads(N, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	gOutput[dtid.x] = length(gInput[dtid.x].v);
}