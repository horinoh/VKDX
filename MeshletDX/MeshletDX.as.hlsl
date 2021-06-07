struct PAYLOAD_OUT
{
    uint InstanceID;
    uint MeshletDimension;
    uint MeshletIDs[32];
};

groupshared PAYLOAD_OUT Payload;

#define N 5
#define NN (N * N)

//!< 可視判定, LOD, テセレーション等, 動的なものにアンプリフィケーションシェーダーを使う、静的なものならメッシュシェーダのみで良い
[numthreads(NN, 1, 1)]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    Payload.InstanceID = GroupID;
    Payload.MeshletDimension = N;

    bool Visible = true;
    //bool Visible = 0 == (GroupThreadID % 2);
    //bool Visible = 1 == (GroupThreadID % 2);

    //!< Wave命令はスレッド間同期される
    
    if (Visible)
    {
        //!< WavePrefixCountBits() : true を返す(より若い)スレッド数を返す、結果的に true を返すスレッド ID が前詰めで格納されていく
        Payload.MeshletIDs[WavePrefixCountBits(Visible)] = GroupThreadID;
    }

    //!< WaveActiveCountBits() : true を返す全スレッド数を返す、その分(メッシュレット数)だけメッシュシェーダを起動する(DispatchMesh())
    DispatchMesh(WaveActiveCountBits(Visible), 1, 1, Payload);
}
