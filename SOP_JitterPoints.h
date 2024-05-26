#ifndef __SOP_JitterPoints_h__
#define __SOP_JitterPoints_h__

#include <SOP/SOP_Node.h>

namespace HDK_Sample
{
    class SOP_JitterPoints : public SOP_Node
    {
    public:
        static OP_Node *myConstructor(OP_Network *, const char *,
                                      OP_Operator *);

        static PRM_Template myTemplateList[];

    protected:
        SOP_JitterPoints(OP_Network *net, const char *name, OP_Operator *op);
        ~SOP_JitterPoints() override;

        const char *inputLabel(unsigned idx) const override;

        OP_ERROR cookMySop(OP_Context &context) override;

        void applyJitter(GU_Detail *dst, OP_Context &context);

    private:
        fpreal getAmount(fpreal t) { return evalFloat("amount", 0, t); }
        int getSeed(fpreal t) { return evalInt("seed", 0, t); }
    };
}

#endif
