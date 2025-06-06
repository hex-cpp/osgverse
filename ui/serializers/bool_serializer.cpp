#include "../SerializerInterface.h"
using namespace osgVerse;

class BoolSerializerInterface : public SerializerInterface
{
public:
    BoolSerializerInterface(osg::Object* obj, LibraryEntry* entry, const LibraryEntry::Property& prop)
        : SerializerInterface(obj, entry, prop, false)
    {
        _check = new CheckBox(TR(_property.name) + _postfix, false);
        _check->tooltip = tooltip(_property);
        _check->callback = [this](ImGuiManager*, ImGuiContentHandler*, ImGuiComponentBase*)
        { if (_entry->setProperty(_object.get(), _property.name, _check->value)) doneEditing(); };
    }

    virtual bool showProperty(ImGuiManager* mgr, ImGuiContentHandler* content)
    {
        if (isDirty()) _entry->getProperty(_object.get(), _property.name, _check->value);
        return _check->show(mgr, content);
    }

protected:
    osg::ref_ptr<CheckBox> _check;
};

REGISTER_SERIALIZER_INTERFACE(BOOL, BoolSerializerInterface)
