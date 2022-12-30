/*
 * Compute Shader는 길이가 [1,10] 구간인 3차원 벡터 64개를 담은 구조적 버퍼를 입력받아서 그 벡터들의 길이를 계산하고,
 * 계산 결과를 하나의 부동소수점 버퍼에 출력한다.
 * 응용프로그램은 그 결과를 시스템 메모리에 복사해서 파일에 저장한다.
 * 실행하고 파일에 기록된 모든 길이가 실제로 [1,10] 구간인지 검증하라.
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