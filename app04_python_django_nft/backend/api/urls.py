from django.urls import path

from api import views

urlpatterns = [
    path("uploads", views.UploadView.as_view()),
    path("artworks", views.ArtworkListView.as_view()),
    path("artworks/<str:artwork_id>/mint", views.ArtworkMintView.as_view()),
    path("certificates/<str:token_id_raw>", views.CertificateDetailView.as_view()),
    path("certificates/<str:token_id_raw>/pdf", views.CertificatePdfView.as_view()),
    path("artists/<str:wallet_address>/dashboard", views.ArtistDashboardView.as_view()),
    path("i18n/<str:lang>", views.I18nView.as_view()),
    path("identity/verify", views.IdentityVerifyView.as_view()),
]
