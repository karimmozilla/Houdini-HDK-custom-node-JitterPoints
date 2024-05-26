#include <random>

#include "SOP_JitterPoints.h"

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_AutoLockInputs.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>

using namespace HDK_Sample;

void newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        "hdk_jitterpoints",
        "Jitter Points",
        SOP_JitterPoints::myConstructor,
        SOP_JitterPoints::myTemplateList,
        1,
        1,
        0,
        OP_FLAG_GENERATOR,
        0,
        1));
}

static PRM_Name prmJitterAmount("amount", "Jitter Amount");
static PRM_Name prmJitterSeed("seed", "Jitter Seed");

PRM_Template SOP_JitterPoints::myTemplateList[] = {
    PRM_Template(PRM_FLT_J, 1, &prmJitterAmount, PRMzeroDefaults, 0, &PRMdivision1Range),
    PRM_Template(PRM_INT, 1, &prmJitterSeed, PRMoneDefaults, 0, &PRMdivision1Range),
    PRM_Template()};

OP_Node *SOP_JitterPoints::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_JitterPoints(net, name, op);
}

SOP_JitterPoints::SOP_JitterPoints(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op)
{
    mySopFlags.setManagesDataIDs(true);
}

SOP_JitterPoints::~SOP_JitterPoints() {}

OP_ERROR SOP_JitterPoints::cookMySop(OP_Context &context)
{
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    duplicateSource(0, context);
    applyJitter(gdp, context);

    return error();
}

void SOP_JitterPoints::applyJitter(GU_Detail *gdp, OP_Context &context)
{
    fpreal now = context.getTime();
    float jitterAmount = getAmount(now) / 10;
    int jitterSeed = getSeed(now);

    UT_Array<GA_RWHandleV3> positionAttribs(1);
    GA_Attribute *attrib;
    GA_FOR_ALL_POINT_ATTRIBUTES(gdp, attrib)
    {
        if (!attrib->needsTransform())
            continue;

        GA_TypeInfo typeInfo = attrib->getTypeInfo();
        if (typeInfo == GA_TYPE_POINT || typeInfo == GA_TYPE_HPOINT)
        {
            GA_RWHandleV3 handle(attrib);
            if (handle.isValid())
            {
                positionAttribs.append(handle);
                attrib->bumpDataId();
            }
        }
    }

    GA_Offset start, end;
    for (GA_Iterator it(gdp->getPointRange()); it.blockAdvance(start, end);)
    {
        for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
        {
            for (exint i = 0; i < positionAttribs.size(); ++i)
            {
                UT_Vector3 position = positionAttribs(i).get(ptoff);

                int frameNumber = static_cast<int>(context.getFrame());
                std::seed_seq seedSeq{jitterSeed * frameNumber};
                std::mt19937 rng(seedSeq);
                std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                float jitterFactor = dist(rng);

                position.x() += jitterFactor * jitterAmount;
                position.y() += jitterFactor * jitterAmount;
                position.z() += jitterFactor * jitterAmount;

                positionAttribs(i).set(ptoff, position);
            }
        }
    }
}

const char *SOP_JitterPoints::inputLabel(unsigned) const
{
    return "Geometry to Jitter";
}
