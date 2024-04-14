//!< 最新の dxc をダウンロードして解凍
//!< https://github.com/microsoft/DirectXShaderCompiler/releases
//!< dxc.exe を使用してコンパイル
//!< $dxc.exe -E main -T lib_6_8 -Fo XXX.sco XXX.hlsl

struct EntryRecord
{
    uint GridSize : SV_DispatchGrid;
    uint RecordIndex;
};

struct SecondNodeInput
{
    uint EntryRecordIndex;
    uint IncrementValue;
};

//!< Work Graphs では "node"
[Shader("node")]
//!< "broadcasting", "coalescing", "thread" がある
[NodeLaunch("broadcasting")]
//!< DispatchGraph 引数の最大値
[NodeMaxDispatchGrid(16, 1, 1)]
//!< スレッドグループのスレッド数
[NumThreads(2, 1, 1)]
void main(DispatchNodeInputRecord<EntryRecord> InputData, [MaxRecords(2)] NodeOutput<SecondNodeInput> SecondNode, uint ThreadIndex : SV_GroupIndex, uint DispatchThreadID : SV_DispatchThreadID)
{
    //!< 出力用の確保、スレッドグループ単位で実行される
    GroupNodeOutputRecords<SecondNodeInput> Out = SecondNode.GetGroupNodeOutputRecords(2);

    Out[ThreadIndex].EntryRecordIndex = InputData.Get().RecordIndex;
    Out[ThreadIndex].IncrementValue = DispatchThreadID * 2 + ThreadIndex + 1;

    //!< スレッドグループ単位で実行される
    Out.OutputComplete();
}

[Shader("node")]
[NodeLaunch("thread")]
void SecondNode(
    ThreadNodeInputRecord<SecondNodeInput> inputData
    //,[MaxRecords(1)] NodeOutput<thirdNodeInput> thirdNode
)
{
    // UAV[entryRecordIndex] (as uint) is the sum of all outputs from upstream node for graph entry [entryRecordIndex]
    //InterlockedAdd(UAV[inputData.Get().EntryRecordIndex], inputData.Get().IncrementValue);

    // For every thread send a task to thirdNode
   // ThreadNodeOutputRecords<thirdNodeInput> outRec = thirdNode.GetThreadNodeOutputRecords(1);
    //outRec.Get().entryRecordIndex = inputData.Get().entryRecordIndex;
    //outRec.OutputComplete();
}