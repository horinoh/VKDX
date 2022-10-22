struct IN
{
    uint InstanceID : SV_InstanceID;
};

struct OUT
{
    uint InstanceID : SV_InstanceID;
};

OUT main(IN In)
{
    OUT Out;
    Out.InstanceID = In.InstanceID;
    return Out;
}
