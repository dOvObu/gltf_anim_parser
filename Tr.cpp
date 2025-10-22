#include "Tr.h"







void Tr::ApplyTo(const Shader &shader, const std::string &parameterId)
{
    SendToGPU(shader.GetUniformId(parameterId));
}

void Tr::SendToGPU(int uniformId)
{
    glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(_mat));
}







void Tr::RecalcMat(bool recursively)
{
    _mat = glm::translate(glm::mat4(1), _locPos) * glm::mat4_cast(_locRot) * glm::scale(glm::mat4(1), _locScale);

    if (_parent != nullptr)
    {
        _mat = _parent->_mat * _mat;
    }

    if (!recursively)
    {
        return;
    }

    for (Tr* ch : _children)
    {
        ch->RecalcMat();
    }
}







Tr *Tr::SetWPosition(glm::vec3 p)
{
    if (_parent == nullptr)
    {
        SetLPosition(p);
        return this;
    }
    _locPos = glm::inverse(_parent->_mat) * glm::vec4(p, 1);
    RecalcMat();
    return this;
}

glm::vec3 Tr::GetWPosition() const
{
    return _parent == nullptr
           ? _locPos
           : _parent->_mat * glm::vec4(_locPos, 1);
}

Tr *Tr::SetLPosition(glm::vec3 p)
{
    _locPos = p;
    RecalcMat();
    return this;
}

glm::vec3 Tr::GetLPosition() const
{
    return _locPos;
}







Tr *Tr::SetWRotation(glm::quat r)
{
    if (_parent == nullptr)
    {
        return SetLRotation(r);
    }
    _locRot = glm::inverse(glm::quat_cast(_parent->_mat)) * r;
    RecalcMat();
    return this;
}

glm::quat Tr::GetWRotation() const
{
    if (_parent == nullptr)
    {
        return _locRot;
    }
    return glm::quat_cast(_parent->_mat) * _locRot;
}

Tr *Tr::SetLRotation(glm::quat r)
{
    _locRot = r;RecalcMat();
    return this;
}

glm::quat Tr::GetLRotation() const
{
    return _locRot;
}







Tr *Tr::SetWEuler(glm::vec3 r)
{
    SetWRotation(glm::quat(r));
    return this;
}

glm::vec3 Tr::GetWEuler() const
{
    return glm::eulerAngles(GetWRotation());
}

Tr *Tr::SetLEuler(glm::vec3 r)
{
    SetLRotation(glm::quat(r));
    return this;
}

glm::vec3 Tr::GetLEuler() const
{
    return glm::eulerAngles(_locRot);
}







Tr *Tr::SetWScale(glm::vec3 s)
{
    if (_parent == nullptr)
    {
        SetLScale(s);
        return this;
    }
    glm::vec3 lossyScale = _parent->GetWScale();
    _locScale = glm::vec3
    {
        s.x / lossyScale.x,
        s.y / lossyScale.y,
        s.z / lossyScale.z
    };
    RecalcMat();
    return this;
}

glm::vec3 Tr::GetWScale() const
{
    return glm::vec3
           {
               glm::column(_mat, 0).length(),
               glm::column(_mat, 1).length(),
               glm::column(_mat, 2).length()
           };
}

Tr *Tr::SetLScale(glm::vec3 s)
{
    _locScale = s;
    RecalcMat();
    return this;
}

glm::vec3 Tr::GetLScale() const
{
    return _locScale;
}







void Tr::SetParent(Tr *transform, bool recalcMatRecursively)
{
    glm::vec3 lossyScale = _parent != nullptr
            ? _parent->GetWScale()
            : glm::vec3(1);

    if (_parent != nullptr)
    {
        _parent->_children.erase(std::remove(_parent->_children.begin(), _parent->_children.end(), this));
    }

    glm::quat rot = GetWRotation();
    glm::vec3 pos = GetWPosition();
    glm::vec3 sca = glm::vec3
        {
            _locScale.x * lossyScale.x,
            _locScale.y * lossyScale.y,
            _locScale.z * lossyScale.z
        };

    _parent = transform;

    if (transform == nullptr)
    {
        _locPos = pos;
        _locRot = rot;
        _locScale = sca;
    }
    else
    {
        _locPos = glm::inverse(_parent->_mat) * glm::vec4(pos, 1);
        _locRot = glm::inverse(glm::quat_cast(_parent->_mat)) * rot;
        _locScale = glm::vec3
        {
            sca.x / lossyScale.x,
            sca.y / lossyScale.y,
            sca.z / lossyScale.z
        };
        _parent->_children.push_back(this);
    }

    RecalcMat(recalcMatRecursively);
}

void Tr::SetParentKeepingLocalTransform(Tr* transform)
{
    if (_parent != nullptr)
    {
        _parent->_children.erase(std::remove(_parent->_children.begin(), _parent->_children.end(), this));
    }
    _parent = transform;

    if (_parent != nullptr)
    {
        _parent->_children.push_back(this);
    }
}







Tr *::Tr::Q(const std::string &name)
{
    return Q([&name](Tr* p) -> bool { return name == p->_name; });
}

std::vector<Tr *> Tr::Query(const std::string &name)
{
    return Query([&name](Tr* p) -> bool { return name == p->_name; });
}

Tr *Tr::Q(std::function<bool(Tr *)> p)
{
    if (p(this))
    {
        return this;
    }
    std::set<Tr*> items(std::begin(_children), std::end(_children));
    while (!items.empty())
    {
        std::set<Tr*> transforms(std::begin(items), std::end(items));
        items = {};

        for (Tr* tr : transforms)
        {
            if (p(tr))
            {
                return tr;
            }
            for (Tr* ch : tr->_children)
            {
                items.emplace(ch);
            }
        }
    }
    return nullptr;
}

std::vector<Tr *> Tr::Query(std::function<bool(Tr *)> p)
{
    std::vector<Tr*> result;
    if (p(this))
    {
        result.push_back(this);
    }
    std::set<Tr*> items(std::begin(_children), std::end(_children));
    while (!items.empty())
    {
        std::set<Tr*> transforms(std::begin(items), std::end(items));
        items = {};

        for (Tr* tr : transforms)
        {
            if (p(tr))
            {
                result.push_back(tr);
            }
            for (Tr* ch : tr->_children)
            {
                items.emplace(ch);
            }
        }
    }
    return result;
}

void Tr::SetLPositionWithoutMatrixRecalc(glm::vec3 p)
{
    _locPos = p;
}

void Tr::SetLRotationWithoutMatrixRecalc(glm::quat r)
{
    _locRot = r;
}

void Tr::SetLScaleWithoutMatrixRecalc(glm::vec3 s)
{
    _locScale = s;
}