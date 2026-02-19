#pragma once

#include <string>

#include <cstdint>
#include <nlohmann/json.hpp>

namespace cyrex::nw::protocol
{

enum class authenticationType : std::int32_t
{
    FULL,
    GUESS,
    SELF_SIGNED
};

// NOLINTBEGIN(readability-identifier-naming)
struct ClientDataAnimationFrame
{
    std::int32_t ImageHeight{};
    std::int32_t ImageWidth{};
    float Frames{};
    std::int32_t Type{};
    std::string Image;
    std::int32_t AnimationExpression{};
};

struct ClientDataPersonaSkinPiece
{
    std::string PieceId;
    std::string PieceType;
    std::string PackId;
    bool IsDefault{};
    std::string ProductId;
};

struct ClientDataPersonaPieceTintColor
{
    std::string PieceType;
    std::string Colors;
};

struct NetworkSkinData
{
    std::vector<ClientDataAnimationFrame> AnimatedImageData;
    std::string ArmSize;
    std::string CapeData;
    std::string CapeId;
    std::int32_t CapeImageHeight{};
    std::int32_t CapeImageWidth{};
    bool CapeOnClassicSkin{};
    std::int64_t ClientRandomId{};
    bool CompatibleWithClientSideChunkGen{};
    std::int32_t CurrentInputMode{};
    std::int32_t DefaultInputMode{};
    std::string DeviceId;
    std::string DeviceModel;
    std::int32_t DeviceOS{};
    std::string GameVersion;
    std::int32_t GraphicsMode{};
    std::int32_t GuiScale{};
    bool IsEditorMode{};
    std::string LanguageCode;
    std::int32_t MaxViewDistance{};
    std::int32_t MemoryTier{};
    bool OverrideSkin{};
    std::vector<ClientDataPersonaSkinPiece> PersonaPieces;
    bool PersonaSkin{};
    std::vector<ClientDataPersonaPieceTintColor> PieceTintColors;
    std::string PlatformOfflineId;
    std::string PlatformOnlineId;
    std::int32_t PlatformType{};
    bool PremiumSkin{};
    std::string SelfSignedId;
    std::string ServerAddress;
    std::string SkinAnimationData;
    std::string SkinColor;
    std::string SkinData;
    std::string SkinGeometryData;
    std::string SkinGeometryDataEngineVersion;
    std::string SkinId;
    std::int32_t SkinImageHeight{};
    std::int32_t SkinImageWidth{};
    std::string SkinResourcePatch;
    std::string ThirdPartyName;
    bool TrustedSkin{};
    std::int32_t UIProfile{};
};

struct AuthenticationInfo
{
    authenticationType AuthenticationType{};
    std::string Certificate;
    std::string Token;
};

struct AuthIdentityData
{
    std::string displayName;
    std::string identity;
    std::string sandboxId;
    std::string titleId;
    std::string XUID;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientDataAnimationFrame, // NOLINT
    ImageHeight,
    ImageWidth,
    Frames,
    Type,
    Image,
    AnimationExpression
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientDataPersonaSkinPiece, // NOLINT
    PieceId,
    PieceType,
    PackId,
    IsDefault,
    ProductId
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientDataPersonaPieceTintColor, // NOLINT
    PieceType,
    Colors
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkSkinData, // NOLINT
    AnimatedImageData,
    ArmSize,
    CapeData,
    CapeId,
    CapeImageHeight,
    CapeImageWidth,
    CapeOnClassicSkin,
    ClientRandomId,
    CompatibleWithClientSideChunkGen,
    CurrentInputMode,
    DefaultInputMode,
    DeviceId,
    DeviceModel,
    DeviceOS,
    GameVersion,
    GraphicsMode,
    GuiScale,
    IsEditorMode,
    LanguageCode,
    MaxViewDistance,
    MemoryTier,
    OverrideSkin,
    PersonaPieces,
    PersonaSkin,
    PieceTintColors,
    PlatformOfflineId,
    PlatformOnlineId,
    PlatformType,
    PremiumSkin,
    SelfSignedId,
    ServerAddress,
    SkinAnimationData,
    SkinColor,
    SkinData,
    SkinGeometryData,
    SkinGeometryDataEngineVersion,
    SkinId,
    SkinImageHeight,
    SkinImageWidth,
    SkinResourcePatch,
    ThirdPartyName,
    TrustedSkin,
    UIProfile
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthenticationInfo, // NOLINT
    AuthenticationType,
    Certificate,
    Token
);
// NOLINTEND(readability-identifier-naming)

} // namespace cyrex::nw::protocol