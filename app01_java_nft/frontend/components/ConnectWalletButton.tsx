'use client';

import { useAccount, useConnect, useDisconnect } from 'wagmi';

function shortenAddress(address: string): string {
  return `${address.slice(0, 6)}…${address.slice(-4)}`;
}

export function ConnectWalletButton() {
  const { address, isConnected } = useAccount();
  const { connectors, connect, isPending } = useConnect();
  const { disconnect } = useDisconnect();

  if (isConnected && address) {
    return (
      <div className="flex items-center gap-2">
        <span className="rounded-full bg-ink/5 px-3 py-1 text-sm font-medium text-ink">
          {shortenAddress(address)}
        </span>
        <button
          type="button"
          onClick={() => disconnect()}
          className="rounded-full border border-ink/20 px-3 py-1 text-sm text-ink/70 hover:bg-ink/5"
        >
          Disconnect
        </button>
      </div>
    );
  }

  const primaryConnector = connectors[0];

  return (
    <button
      type="button"
      disabled={!primaryConnector || isPending}
      onClick={() => primaryConnector && connect({ connector: primaryConnector })}
      className="rounded-full bg-brass px-4 py-1.5 text-sm font-semibold text-white hover:bg-brass/90 disabled:opacity-50"
    >
      {isPending ? 'Connecting…' : 'Connect Wallet'}
    </button>
  );
}
