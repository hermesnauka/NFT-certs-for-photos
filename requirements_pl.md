

# ROLA 1: ZGŁOŚ SIĘ JAKO GŁÓWNY ARCHITEKT WEB3 I INŻYNIER SMART KONTRAKTÓW (LEAD WEB3 & SMART CONTRACT ARCHITECT)

## CEL
Zaprojektuj i zaimplementuj architekturę backendową oraz Smart Kontrakty dla legalnej platformy tokenizacji dzieł sztuki (fotografii artystycznej). System musi przyjmować wysokiej jakości zdjęcia, generować dla nich kryptograficzne dowody autentyczności, a następnie tworzyć (mintować) certyfikaty NFT przygotowane do sprzedaży na publicznych rynkach (np. OpenSea, Rarible, Blur), z bezwzględnym naciskiem na ochronę praw autorskich twórcy.
Aplikacja musi być z przełącznikiem, aby wyświetlać ją w języku polskim lub w angielskim za pomocą prostego przełącznika z flagą.

## WYMAGANIA IMPLEMENTACYJNE:
1. **Ochrona Treści Cyfrowych i Autentyczność:**
   - Zaimplementuj mechanizm kryptograficznego haszowania plików zdjęciowych (np. SHA-256) przed ich wysłaniem do zdecentralizowanego magazynu.
   - Opracuj proces "cyfrowego znaku wodnego" lub steganografii połączonej z metadanymi, aby w sposób bezsporny powiązać plik z tożsamością artysty.
2. **Zdecentralizowany Storage (IPFS / Arweave):**
   - Skonfiguruj konektory do trwałego, rozproszonego przechowywania plików źródłowych oraz metadanych (JSON) na IPFS lub Arweave, gwarantując niezmienność dzieła (immutable URLs).
3. **Smart Kontrakty (Solidity / Java Web3j):**
   - Zaimplementuj i zoptymalizuj kontrakty w standardzie ERC-721 lub ERC-1155.
   - Wdróż standard **EIP-2981 (NFT Royalty Standard)**, aby prawnie i technicznie zabezpieczyć tantiemy (royalties) dla twórców ze sprzedaży wtórnej na wszystkich publicznych rynkach.
   - Dodaj mechanizmy wstrzymania (Pausable) lub spalenia (Burnable) w przypadku prawnych roszczeń dotyczących praw autorskich, przy zachowaniu zasady decentralizacji.
4. **Tożsamość Twórcy (DID i KYC):**
   - Zaprojektuj przepływ weryfikacji tożsamości artysty (Decentralized Identifiers - DID lub integracja z dostawcami KYC), aby potwierdzić, że osoba mintująca jest faktycznym właścicielem praw do zdjęcia.

## FORMAT I KONEKTORY
Wylistuj niezbędne konektory (np. IPFS HTTP Client, Web3j / Ethers.js, Pinata API). Dostarcz gotowy kod Smart Kontraktów wraz z obszernymi testami jednostkowymi. Używaj profesjonalnego nazewnictwa inżynieryjnego.
Aplikacja musi być z przełącznikiem, aby wyświetlać ją w języku polskim lub w angielskim za pomocą prostego przełącznika z flagą.


#


# ROLA 2: ZGŁOŚ SIĘ JAKO GŁÓWNY PROGRAMISTA FRONTEND DAPP I SPECJALISTA UX/UI (LEAD DAPP FRONTEND & UX/UI ENGINEER)

## CEL
Zbuduj intuicyjny, bezpieczny i profesjonalny portal dla artystów-fotografików (Creator Portal - Frontend w React/Next.js). Aplikacja ma służyć jako bezpieczna brama do załadowania zdjęć artystycznych, generowania dla nich certyfikatów NFT oraz zarządzania ich sprzedażą na publicznych rynkach. Frontend musi edukować twórcę o jego prawach i transparentnie pokazywać proces zabezpieczania dzieła.
Aplikacja musi być z przełącznikiem, aby wyświetlać ją w języku polskim lub w angielskim za pomocą prostego przełącznika z flagą.

## WYMAGANIA FUNKCJONALNE FRONTENDU:
1. **Zarządzanie Portfelem (Web3 Integration):**
   - Zaimplementuj bezpieczne łączenie portfeli (np. za pomocą WalletConnect, MetaMask, Coinbase Wallet).
   - Stwórz przejrzysty Dashboard pokazujący zasoby artysty, zarobione tantiemy (royalties) oraz historię własności wygenerowanych certyfikatów.
2. **Kreator Certyfikatów (Minting Interface):**
   - Zaprojektuj formularz wgrywania zdjęcia ("Drag & Drop"), który w czasie rzeczywistym generuje i wyświetla użytkownikowi hasz pliku (np. SHA-256), edukując go, że to jest "cyfrowy odcisk palca" chroniący jego dzieło.
   - Umożliw zdefiniowanie metadanych: Tytuł, Opis, Technika, Rok wykonania, Procent Tantiem (Royalties).
3. **Wizualizacja Bezpieczeństwa i Praw Autorskich:**
   - Wyświetlaj generowany cyfrowy certyfikat własności w formie eleganckiego, możliwego do pobrania (np. do PDF) dokumentu, który zawiera linki do transakcji na blockchainie (Etherscan) oraz linki IPFS.
   - Dodaj moduł informacyjny, który przystępnym językiem tłumaczy artyście, w jaki sposób Smart Kontrakt gwarantuje autentyczność i chroni przed piractwem na rynkach zewnętrznych.
4. **Integracja z Rynkami (Public Marketplaces):**
   - Zaimplementuj API lub głębokie linkowanie (deep links), które pozwala po udanym procesie mintowania wygenerować bezpośrednie przyciski: "Wystaw na OpenSea", "Zobacz na Rarible", weryfikujące obecność kontraktu na tych platformach.

## OCZEKIWANY WYNIK
Zwróć architekturę drzewa komponentów (np. w oparciu o tailwindcss i wagmi/viem dla Reacta). Napisz kod kluczowych komponentów odpowiadających za proces podpisywania transakcji oraz wgrywania plików na IPFS. Zachowaj rygor bezpieczeństwa danych po stronie klienta.
Aplikacja musi być z przełącznikiem, aby wyświetlać ją w języku polskim lub w angielskim za pomocą prostego przełącznika z flagą.

