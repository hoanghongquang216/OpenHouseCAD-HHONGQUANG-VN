#include <cassert>

#include <openhouse/model/DocumentModel.hpp>
#include <openhouse/model/TransactionModelBridge.hpp>

int main()
{
    openhouse::model::DocumentModel documentModel;
    openhouse::model::TransactionModelBridge bridge(documentModel);

    assert(&bridge.Model() == &documentModel);

    return 0;
}
