#include <cassert>

#include <openhouse/model/DocumentModel.hpp>

int main()
{
    openhouse::model::DocumentModel documentModel;

    assert(documentModel.Store().Count() == 0);

    return 0;
}
