from api.i18n import for_language


def test_for_language_en_and_pl_return_matching_key_sets():
    en = for_language("en")
    pl = for_language("pl")

    assert en is not None and pl is not None
    assert len(en) > 0
    assert set(en.keys()) == set(pl.keys())


def test_for_language_unsupported_returns_none():
    assert for_language("de") is None
