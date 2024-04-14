//!< �ŐV�� dxc ���_�E�����[�h���ĉ�
//!< https://github.com/microsoft/DirectXShaderCompiler/releases
//!< dxc.exe ���g�p���ăR���p�C��
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

//!< Work Graphs �ł� "node"
[Shader("node")]
//!< "broadcasting", "coalescing", "thread" ������
[NodeLaunch("broadcasting")]
//!< DispatchGraph �����̍ő�l
[NodeMaxDispatchGrid(16, 1, 1)]
//!< �X���b�h�O���[�v�̃X���b�h��
[NumThreads(2, 1, 1)]
void main(DispatchNodeInputRecord<EntryRecord> InputData, [MaxRecords(2)] NodeOutput<SecondNodeInput> SecondNode, uint ThreadIndex : SV_GroupIndex, uint DispatchThreadID : SV_DispatchThreadID)
{
    //!< �o�͗p�̊m�ہA�X���b�h�O���[�v�P�ʂŎ��s�����
    GroupNodeOutputRecords<SecondNodeInput> Out = SecondNode.GetGroupNodeOutputRecords(2);

    Out[ThreadIndex].EntryRecordIndex = InputData.Get().RecordIndex;
    Out[ThreadIndex].IncrementValue = DispatchThreadID * 2 + ThreadIndex + 1;

    //!< �X���b�h�O���[�v�P�ʂŎ��s�����
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